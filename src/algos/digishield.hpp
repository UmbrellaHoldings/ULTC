// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * The DigiShield algo.
 *
 * @author l0rdicon <xploited.ca@gmail.com>
 * @author digibyte <dev@digibyte.co>
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_DIGISHIELD_HPP
#define BITCOIN_DIGISHIELD_HPP

#include "digishield.h"
#include "main.h"
#include "block.h"
#include "types/exception.h"

using namespace types;
using namespace coin;
using namespace coin::except;

namespace DigiByte {

template<class Block>
bool difficulty::is_valid(const Block& block)
{
  const auto diff = block.nBits;

  return diff > 0 && diff <= min_difficulty_by_design;
}

template<class Block>
compact_bignum_t difficulty::dos_min_difficulty(
  const Block& block
) const
{
  const auto* last_reliable_block =
    dos_last_reliable_block();

  const auto past = block.GetTimePoint() 
    - last_reliable_block->GetTimePoint();

  if (past < times::block::zero)
    throw exception<invalid_timestamp>(
      "block with timestamp before last checkpoint"
  );
  
  return std::min(
    CBigNum(last_reliable_block->nBits)
    * past / block_period_by_design
    * adjustment_by_design,
    CBigNum(min_difficulty_by_design)
  ).GetCompact();
}

template<class Block>
void difficulty
::dos_check_min_difficulty(const Block& block) const
{
  if (block.nBits.lower_difficulty
      (dos_min_difficulty(block))
      )
    throw exception<invalid_difficulty>(
      "block with too little proof-of-work"
    );
}

} // DigiByte

#endif
