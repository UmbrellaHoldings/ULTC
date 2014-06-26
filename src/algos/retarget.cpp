// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * A retarget algo abstraction
 *
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#include <boost/thread.hpp>
#include "types/fixed.h"
#include "algos/retarget.h"
#include "log.h"
#include "util.h"
#include "pars.h"
#include "btc_time.h"
#include "checkpoints.h"
#include "main.h"

using namespace types;

extern std::map<uint256, CBlockIndex*> mapBlockIndex;

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
    duration desired_timespan,
    duration actual_timespan,
    compact_bignum_t last_block_difficulty
  ) override
  {
    return (
      (actual_timespan < desired_timespan)
        ? last_block_difficulty >> 1
        : last_block_difficulty << 1
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
    duration desired_timespan,
    duration actual_timespan,
    compact_bignum_t last_block_difficulty
  ) override
  {
    auto nActualTimespan = actual_timespan;

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
    pindexLast->GetTimePoint() 
      - pindexFirst->GetTimePoint(),
    pindexLast->nBits
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
