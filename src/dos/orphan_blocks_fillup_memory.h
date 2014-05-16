// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Do not accept blocks with too low difficulty.
 * See also https://bitcointalk.org/index.php?topic=23266.0
 *
 * @author Gavin Andresen <gavinandresen@gmail.com>
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_DOS_ORPHAN_BLOCKS_FILLUP_MEMORY_H
#define BITCOIN_DOS_ORPHAN_BLOCKS_FILLUP_MEMORY_H

namespace dos {

namespace orphan_blocks_fillup_memory {

//! Suppose we are nTime after the last
//! checkpoint. Compute min difficulty allowed based on
//! the last checkpointed block difficulty which is
//! nBase. It is nBase *= adjustment * nTime / nTargetTime
template<fixed_t adjustment>
unsigned int ComputeMinWork(unsigned int nBase, int64 nTime)
{
  CBigNum bnResult;
  bnResult.SetCompact(nBase);

#if 1
  bnResult = std::min(
    bnResult * adjustment_percent * nTime / nTargetTime,
    bnProofOfWorkLimit
  );
#else
  while (nTime > 0 && bnResult < bnProofOfWorkLimit)
  {
    // Maximum 10% adjustment...
    bnResult = (bnResult * 110) / 100;
    // ... in best-case exactly 4-times-normal target time
    nTime -= nTargetTimespan*4;
  }
  if (bnResult > bnProofOfWorkLimit)
    bnResult = bnProofOfWorkLimit;
#endif
    
  return bnResult.GetCompact();
}

} // orphan_blocks_fillup_memory

} // dos

#endif


