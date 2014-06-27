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

using namespace types;

extern std::map<uint256, CBlockIndex*> mapBlockIndex;
extern CBlockIndex* pindexGenesisBlock;

namespace retarget {

template<pars::retarget_algo>
class difficulty_impl;

//! Allows change difficulty only by 2 or 1/2
template<>
class difficulty_impl<pars::retarget_algo::twice_and_half>
  : public difficulty
{
public:
  compact_bignum_t next_block_difficulty(
    const duration desired_timespan,
    const block_info_iterator_base_t& 
      last_blocks_info_rbegin,
    const block_info_iterator_base_t& 
      last_blocks_info_rend
  ) override
  {
    if (last_blocks_info_rbegin == last_blocks_info_rend)
      return min_difficulty_by_design;

    assert(
      height_diff(
        last_blocks_info_rbegin, 
        last_blocks_info_rend
      ) > 0
    );

    auto prev = last_blocks_info_rbegin;
    ++prev;
    const auto actual_timespan = 
      last_blocks_info_rbegin->time - prev->time;
    return (
      (actual_timespan < desired_timespan)
        ? last_blocks_info_rbegin->difficulty >> 1
        : last_blocks_info_rbegin->difficulty << 1
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
class difficulty_impl<pars::retarget_algo::digishield>
  : public difficulty
{
public:
  //! The limit parameter for dos_min_difficulty()
  const coin::percent_t adjustment_by_design = 
    coin::operator"" _pct(110);

  compact_bignum_t next_block_difficulty(
    const duration desired_timespan,
    const block_info_iterator_base_t& 
      last_blocks_info_rbegin,
    const block_info_iterator_base_t& 
      last_blocks_info_rend
  ) override
  {
    if (last_blocks_info_rbegin == last_blocks_info_rend)
      return min_difficulty_by_design;

    auto prev = last_blocks_info_rbegin;
    ++prev;
    auto nActualTimespan = 
      last_blocks_info_rbegin->time - prev->time;

    LOG() << "  nActualTimespan = " 
          << nActualTimespan
          << "  before bounds\n";

    // thanks to RealSolid & WDC for this code 

    //Amplitude Filter by daft27
    nActualTimespan = desired_timespan 
      + (nActualTimespan - desired_timespan)/8;
    
    //Guts of DigiShield Retarget
    if (nActualTimespan < desired_timespan * 3/4)
      nActualTimespan = desired_timespan * 3/4;
      
    if (nActualTimespan > desired_timespan * 3/2) 
      nActualTimespan = desired_timespan * 3/2;

    // Retarget
    const auto last_block_difficulty =
      last_blocks_info_rbegin->difficulty;
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
    if (past < coin::times::block::zero)
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
class difficulty_impl<pars::retarget_algo::kgw>
  : public difficulty
{
public:
  //! The limit parameter for dos_min_difficulty()
  const coin::percent_t adjustment_by_design = 
    coin::operator"" _pct(110);

  // These constantes are got from vertcoin
  constexpr static duration past_min = 
    coin::times::block::hours(6);
  constexpr static duration past_max =
    coin::times::block::days(7);
  const int past_blocks_min = 
    past_min / block_period_by_design;
  const int past_blocks_max = 
    past_max / block_period_by_design;
  //! The timewrap fix.
  //! Retarget every retarget_interval blocks
  const int retarget_interval = 12;

  compact_bignum_t next_block_difficulty(
    const duration desired_timespan,
    const block_info_iterator_base_t& 
      last_blocks_info_rbegin,
    const block_info_iterator_base_t& 
      last_blocks_info_rend
  ) override
  {
    using namespace coin::times::block;

    // TODO don't use floating point
    using float_duration = std::chrono::duration<double>;

    if (last_blocks_info_rbegin->height 
          % retarget_interval != 0
        )
       // the timewrap fix
      return last_blocks_info_rbegin->difficulty;

    // early blocks rule
    if (height_diff(
          last_blocks_info_rbegin,
          last_blocks_info_rend
        ) < past_blocks_min
       )
      return min_difficulty_by_design;

    int64_t mass = 0;
    const auto breaking_el = std::find_if(
      last_blocks_info_rbegin,
      last_blocks_info_rend,
      [
        this,
        desired_timespan,
        &last_blocks_info_rbegin,
        &mass
      ]
        (const block::info_type& bi)
      {
        constexpr auto zero_duration = clock::duration(0);
        if (++mass > past_blocks_max)
          return true;

        const clock::duration desired_passed = 
          desired_timespan * mass;
        const clock::duration actual_passed = std::max(
          zero_duration,
          last_blocks_info_rbegin->time - bi.time
        );

        const double PastRateAdjustmentRatio =
          (actual_passed != zero_duration && 
           desired_passed != zero_duration
           )
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
            * pow((double(mass)/double(144)), 
                  -1.228));

        const double EventHorizonDeviationFast = 
          EventHorizonDeviation;
        const double EventHorizonDeviationSlow = 
          1.0 / EventHorizonDeviation;
                
        if (mass >= past_blocks_min) 
        {
          if ((PastRateAdjustmentRatio <= 
               EventHorizonDeviationSlow) || 
              (PastRateAdjustmentRatio >= 
               EventHorizonDeviationFast)) 
            return true;
        }
        return false;
      }
    );

    if (breaking_el == last_blocks_info_rend ||
        breaking_el->time >= last_blocks_info_rbegin->time
        )
      return min_difficulty_by_design;

    const auto distance = height_diff
      (last_blocks_info_rbegin, breaking_el);
    assert(distance >= 0);

    const CBigNum PastDifficultyAverage = 
      std::accumulate(
        last_blocks_info_rbegin, 
        breaking_el, 
        CBigNum(0),
        [](const CBigNum& acc, const block::info_type& bi)
        {
          return acc + bi.difficulty;
        }
      ) / (distance + 1);

    const auto range_desired_timespan = 
      desired_timespan * (distance + 1);
    const auto range_actual_timespan =
      last_blocks_info_rbegin->time - breaking_el->time;

    constexpr auto zero_duration = clock::duration(0);
    CBigNum bnNew(PastDifficultyAverage);
    if (range_desired_timespan != zero_duration && 
        range_actual_timespan != zero_duration) 
    {
      // actual passed
      bnNew *= to_fixed(range_actual_timespan); 
      // desired
      bnNew /= to_fixed(range_desired_timespan);
    }
    if (bnNew > min_difficulty_by_design)
      bnNew = min_difficulty_by_design;
  
    LOG() 
      << "GetNextWorkRequired [KGW] RETARGET ["
      << height_diff(last_blocks_info_rbegin, breaking_el)
      << " last blocks analysed]"
      << "\ndesired timespan for the range = " 
      << range_desired_timespan
      << "\nactual timespan for the range = " 
      << range_actual_timespan
      << "\nBefore: " 
      << last_blocks_info_rbegin->difficulty << ' '
      << CBigNum(last_blocks_info_rbegin->difficulty)
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
    if (past < coin::times::block::zero)
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

constexpr coin::times::block::clock::duration
difficulty_impl<pars::retarget_algo::kgw>::past_min;

constexpr coin::times::block::clock::duration
difficulty_impl<pars::retarget_algo::kgw>::past_max;

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
