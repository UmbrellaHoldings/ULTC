// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * A retarget algo abstraction
 *
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#include <algorithm>
#include <iterator>
#include <boost/thread.hpp>
#include "types/fixed.h"
#include "types/abstract_iterator.h"
#include "algos/retarget.h"
#include "log.h"
#include "util.h"
#include "pars.h"
#include "btc_time.h"
#include "checkpoints.h"
#include "main.h"
#include "types.h"

using namespace types;

extern std::map<uint256, CBlockIndex*> mapBlockIndex; 
extern CBlockIndex* pindexGenesisBlock;

namespace retarget {

template<class Algo>
class difficulty_impl;

//! Allows change difficulty only by 2 or 1/2
template<>
class difficulty_impl<pars::twice_and_half>
  : public difficulty
{
public:
  compact_bignum_t next_block_difficulty(
    const duration desired_timespan,
    const iterator& rbegin,
    const iterator& rend
  ) override
  {
    if (rbegin == rend)
      return min_difficulty_by_design;

    assert(height_diff(rbegin, rend) > 0);

    auto prev = rbegin;
    ++prev;
    const auto actual_timespan = 
      rbegin->time - prev->time;
    return (
      (actual_timespan < desired_timespan)
        ? rbegin->difficulty >> 1
        : rbegin->difficulty << 1
    ).GetCompact();
  }

  compact_bignum_t dos_min_difficulty(
    compact_bignum_t last_reliable_block_difficulty,
    coin::times::block::clock::duration past
  ) const override
  {
    return min_difficulty_by_design;
  }
};

template<>
class difficulty_impl<pars::digishield>
  : public difficulty
{
public:
  //! The limit parameter for dos_min_difficulty()
  const percent_t adjustment_by_design = 110.0_pct;

  compact_bignum_t next_block_difficulty(
    const duration desired_timespan,
    const iterator& rbegin,
    const iterator& rend
  ) override
  {
    if (rbegin == rend)
      return min_difficulty_by_design;

    auto prev = rbegin;
    ++prev;
    auto nActualTimespan = rbegin->time - prev->time;

    LOG() << "  nActualTimespan = " 
          << nActualTimespan
          << "  before bounds\n";

    // thanks to RealSolid & WDC for this code 
     
    if ( pars::digishield::limit_steps() ) //switch off for development purposes
    {
      //Amplitude Filter by daft27
      nActualTimespan = desired_timespan 
        + (nActualTimespan - desired_timespan)/8;
    
      //Guts of DigiShield Retarget
      if (nActualTimespan < desired_timespan * 3/4)
        nActualTimespan = desired_timespan * 3/4;
      
      if (nActualTimespan > desired_timespan * 3/2) 
        nActualTimespan = desired_timespan * 3/2;
    }

    if ( coin::times::block::zero_duration == nActualTimespan )
    {
        nActualTimespan = coin::times::block::seconds(1);
    }

    // Retarget
    const auto last_block_difficulty =
      rbegin->difficulty;
    CBigNum bnNew = last_block_difficulty;
    bnNew *= to_fixed(nActualTimespan);
    bnNew /= to_fixed(desired_timespan);
  
    if (bnNew > min_difficulty_by_design)
      bnNew = min_difficulty_by_design;

    LOG() 
      << "GetNextWorkRequired [DigiShield] RETARGET \n"
      << "retargetTimespan = " << desired_timespan
      << " nActualTimespan = " << nActualTimespan
      << "\nBefore: " << last_block_difficulty << ' '
      << CBigNum(last_block_difficulty).getuint256()
      << "\nAfter: " << bnNew.GetCompact() << ' '
      << bnNew.getuint256() << '\n';
  
    return bnNew.GetCompact();
  }

  compact_bignum_t dos_min_difficulty(
    compact_bignum_t last_reliable_block_difficulty,
    coin::times::block::clock::duration past
  ) const override
  {
    if (past < coin::times::block::zero_duration)
    throw types::exception<coin::except::invalid_timestamp>(
        "block with timestamp before last checkpoint"
        );
  
    return std::min(
      CBigNum(last_reliable_block_difficulty)
      * past / block_period_by_design
      * adjustment_by_design,
      CBigNum(min_difficulty_by_design)
      ).GetCompact();
  }
};

template<>
class difficulty_impl<pars::kgw> : public difficulty
{
public:
  const int past_blocks_min = 
    pars::kgw::past_min() / block_period_by_design;
  const int past_blocks_max = 
    pars::kgw::past_max() / block_period_by_design;

  compact_bignum_t next_block_difficulty(
    const duration desired_timespan,
    const iterator& rbegin,
    const iterator& rend
  ) override
  {
    using namespace coin::times::block;

    // TODO don't use floating point
    using float_duration = std::chrono::duration<double>;

    // early blocks rule
    if (height_diff(rbegin, rend) < past_blocks_min)
      return min_difficulty_by_design;

    iterator start = rbegin;
    std::advance(start, past_blocks_min - 1);

    int64_t mass = past_blocks_min - 1;
    const auto breaking_el = std::find_if(
      start,
      rend,
      [
        this,
        desired_timespan,
        &rbegin,
        &mass
      ]
        (const block::info_type& bi)
      {
        // != 0: fixes the same-time blocks retargetting
        // exploit
        constexpr auto min_duration = seconds(1);

        if (++mass > past_blocks_max)
          return true;

        const clock::duration desired_passed = 
          desired_timespan * mass;
        const clock::duration actual_passed = 
          // the timewrap fix (part 1)
          (rbegin->time <= bi.time)
            ? min_duration
            : rbegin->time - bi.time;
        

        assert(actual_passed != zero_duration);
        const double PastRateAdjustmentRatio =
          (desired_passed != zero_duration)
        ?
          std::chrono::duration_cast<float_duration>(
            desired_passed
          )
          / std::chrono::duration_cast<float_duration>(
              actual_passed
          ) 
        :
          1.0;

        // TODO tabulate it as fixed point values
        const double EventHorizonDeviation = 
          1 + (0.7084 
            * pow((double(mass)/double(past_blocks_min)), 
                  -1.228));

        const double EventHorizonDeviationFast = 
          EventHorizonDeviation;
        const double EventHorizonDeviationSlow = 
          1.0 / EventHorizonDeviation;
                
        return PastRateAdjustmentRatio <= 
          EventHorizonDeviationSlow 
          || PastRateAdjustmentRatio >= 
          EventHorizonDeviationFast;
      }
    );

    const auto distance = height_diff(rbegin, breaking_el);
    assert(distance >= 0);

    const CBigNum PastDifficultyAverage = 
      std::accumulate(
        rbegin, 
        breaking_el, 
        CBigNum(0),
        [](const CBigNum& acc, const block::info_type& bi)
        {
          return acc + bi.difficulty;
        }
      ) / (distance + 1);

    // != 0: fixes the same-time blocks retargetting
    // exploit
    constexpr auto min_duration = seconds(1);

    const auto range_desired_timespan = 
      desired_timespan * (distance + 1);
    const auto range_actual_timespan =
      // the timewrap fix (part 2)
      (rbegin->time <= breaking_el->time)
      ? min_duration 
      : rbegin->time - breaking_el->time;

    assert(range_actual_timespan != zero_duration);
    CBigNum bnNew(PastDifficultyAverage);
    if (range_desired_timespan != zero_duration)
    {
      // actual time passed
      bnNew *= to_fixed(range_actual_timespan); 
      // desired
      bnNew /= to_fixed(range_desired_timespan);
    }
    if (bnNew > min_difficulty_by_design)
      bnNew = min_difficulty_by_design;
  
    LOG() 
      << "GetNextWorkRequired [KGW] RETARGET ["
      << height_diff(rbegin, breaking_el)
      << " last blocks analysed]"
      << "\navg difficulty: " 
      << PastDifficultyAverage.GetCompact()
      << "\ndesired timespan for the range = " 
      << range_desired_timespan
      << "\nactual timespan for the range = " 
      << range_actual_timespan
      << "\nBefore: " 
      << rbegin->difficulty << ' '
      << CBigNum(rbegin->difficulty)
           . getuint256()
      << "\nAfter: " << bnNew.GetCompact() << ' '
      << bnNew.getuint256() << std::endl;
  
    return bnNew.GetCompact();
  }

  compact_bignum_t dos_min_difficulty(
    compact_bignum_t last_reliable_block_difficulty,
    coin::times::block::clock::duration past
  ) const override
  {
    return min_difficulty_by_design;
  }
};

difficulty& difficulty::instance()
{
  static boost::once_flag of = BOOST_ONCE_INIT;
  static difficulty* instance = nullptr;
  boost::call_once([]()
  { 
    instance = new difficulty_impl
      <pars::retarget_algorithm>();
  }, of);

  assert(instance);
  return *instance;
}

compact_bignum_t difficulty
::next_block_difficulty(const CBlockIndex* pindexLast)
{
  using namespace types;

  // Genesis block
  if (!pindexLast) 
    return min_difficulty_by_design;
  
  // Limit adjustment step
  const auto pindexFirst = pindexLast->pprev;
  if (!pindexFirst)
    return pindexLast->nBits; // slod

  return next_block_difficulty(
    block_period_by_design,
    block::const_iterator<CBlockIndex>(pindexLast),
    block::const_iterator<CBlockIndex>(pindexGenesisBlock)
  );
}

difficulty::difficulty()
  : min_difficulty_by_design(
      pars::testnet_switch(pars::min_difficulty_by_design)
    ),
    block_period_by_design(
      pars::testnet_switch(pars::block_period_by_design)
    )
{
}

const CBlockIndex* difficulty
::dos_last_reliable_block()
{
  static const CBlockIndex* last_checkpointed_block =
    Checkpoints::GetLastCheckpoint(mapBlockIndex);

  if (last_checkpointed_block)
    return last_checkpointed_block;
  else
  {
    assert(pindexGenesisBlock);
    return pindexGenesisBlock;
  }
}

} // retarget
