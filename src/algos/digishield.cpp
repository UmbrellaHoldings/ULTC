// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The DigiShield algo.
 *
 * @author l0rdicon <xploited.ca@gmail.com>
 * @author digibyte <dev@digibyte.co>
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#include <iostream>
#include <map>
#include "uint256.h"
#include "digishield.hpp"
#include "main.h"
#include "checkpoints.h"
#include "bignum.h"

extern std::map<uint256, CBlockIndex*> mapBlockIndex;

namespace DigiByte {

compact_bignum_t difficulty
::next_block_difficulty(const CBlockIndex* pindexLast)
{
//  using namespace std;
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
  nActualTimespan = block_period_by_design 
    + (nActualTimespan - block_period_by_design)/8;
    
  //Guts of DigiShield Retarget
  if (nActualTimespan < block_period_by_design * 3/4)
    nActualTimespan = block_period_by_design * 3/4;
      
  if (nActualTimespan > block_period_by_design * 3/2) 
    nActualTimespan = block_period_by_design * 3/2;

  // Retarget
  CBigNum bnNew = pindexLast->nBits;
  bnNew *= to_fixed(nActualTimespan);
  bnNew /= to_fixed(block_period_by_design);
  
  /// debug print
  std::cout 
    << "GetNextWorkRequired [DigiShield] RETARGET \n"
    << "retargetTimespan = " << block_period_by_design
    << " nActualTimespan = " << nActualTimespan
    << "\nBefore: " << pindexLast->nBits 
    << CBigNum(pindexLast->nBits).getuint256()
    << "\nAfter: " << bnNew.GetCompact()
    << bnNew.getuint256() << '\n';
  
  if (bnNew > min_difficulty_by_design)
    bnNew = min_difficulty_by_design;

  return bnNew.GetCompact();
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

} // DigiByte
