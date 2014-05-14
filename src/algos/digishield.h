// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The DigiShield algo.
 *
 * @author l0rdicon <xploited.ca@gmail.com>
 * @author digibyte <dev@digibyte.co>
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_DIGISHIELD_H
#define BITCOIN_DIGISHIELD_H

#include "types/fixed.h"
#include "btc_time.h"

namespace DigiByte {

template<
  const coin::time::block::duration& block_period,
  const uint256& min_difficulty_
>
unsigned int GetNextWorkRequired(
  const CBlockIndex* pindexLast, 
  const CBlock *pblock
)
{
  const CBigNum min_difficulty(min_difficulty_);

  // Genesis block
  if (!pindexLast) 
    return min_difficulty.GetCompact();
  
  // Limit adjustment step
  const auto pindexFirst = pindexLast->pprev;
  if (!pindexFirst)
    return pindexLast->nBits; // slod

  auto nActualTimespan = pindexLast->GetTimePoint() 
    - pindexFirst->GetTimePoint();

  std::cout << "  nActualTimespan = " 
            << nActualTimespan
            << "  before bounds\n";

  CBigNum bnNew;
  bnNew.SetCompact(pindexLast->nBits);
  
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
  bnNew *= nActualTimespan;
  bnNew /= block_period;
  
  /// debug print
  std::cout 
    << "GetNextWorkRequired [DigiShield] RETARGET \n"
    << "retargetTimespan = " << block_period
    << " nActualTimespan = " << nActualTimespan << '\n';
  printf(
    "Before: %08x %s\n", 
    pindexLast->nBits, 
    CBigNum().SetCompact(pindexLast->nBits).getuint256()
      . ToString().c_str()
  );
  printf(
    "After: %08x %s\n", 
    bnNew.GetCompact(), 
    bnNew.getuint256().ToString().c_str()
  );
  
  if (bnNew > min_difficulty)
    bnNew = min_difficulty;

  return bnNew.GetCompact();
}

}

#endif
