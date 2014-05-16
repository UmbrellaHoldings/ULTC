// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The DigiShield algo.
 *
 * @author l0rdicon <xploited.ca@gmail.com>
 * @author digibyte <dev@digibyte.co>
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#include "digishield.hpp"
#include "main.h"

namespace DigiByte {

compact_bignum_t difficulty
::GetNextWorkRequired(const CBlockIndex* pindexLast)
{
  using namespace types;

  // Genesis block
  if (!pindexLast) 
    return min_difficulty_by_design;
  
  // Limit adjustment step
  const auto pindexFirst = pindexLast->pprev;
  if (!pindexFirst)
    return pindexLast->nBits; // slod

  auto nActualTimespan = pindexLast->GetTimePoint() 
    - pindexFirst->GetTimePoint();

  std::cout << "  nActualTimespan = " 
            << nActualTimespan
            << "  before bounds\n";

  // thanks to RealSolid & WDC for this code 

  //Amplitude Filter by daft27
  nActualTimespan = block_period 
    + (nActualTimespan - block_period)/8;
    
  //Guts of DigiShield Retarget
  if (nActualTimespan < block_period * 3/4)
    nActualTimespan = block_period * 3/4;
      
  if (nActualTimespan > block_period * 3/2) 
    nActualTimespan = block_period * 3/2;

  // Retarget
  CBigNum bnNew = pindexLast->nBits;
  bnNew *= to_fixed(nActualTimespan);
  bnNew /= to_fixed(block_period);
  
  /// debug print
  std::cout 
    << "GetNextWorkRequired [DigiShield] RETARGET \n"
    << "retargetTimespan = " << block_period
    << " nActualTimespan = " << nActualTimespan << '\n';
  printf(
    "Before: %08x %s\n", 
    pindexLast->nBits, 
    CBigNum(pindexLast->nBits).getuint256()
    . ToString().c_str()
    );
  printf(
    "After: %08x %s\n", 
    bnNew.GetCompact(), 
    bnNew.getuint256().ToString().c_str()
    );
  
  if (bnNew > min_difficulty_by_design)
    bnNew = min_difficulty_by_design;

  return bnNew.GetCompact();
}

  const CBlockIndex* difficulty
  ::dos_last_reliable_block()
  {
    static const CBlockIndex* last_checkpointed_block =
      Checkpoints::GetLastCheckpoint();

    if (last_checkpointed_block)
      return last_chekpointed_block;
    else
    {
      assert(pindexGenesisBlock);
      return pindexGenesisBlock;
    }
  }

} // DigiByte
