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

namespace DigiByte {

  template<class Block>
  bool difficulty::is_valid(const Block& block)
  {
    const auto diff = block.nBits;

    return diff > 0 && diff <= min_difficulty_by_design;
  }

  //! Do not accept blocks with too low difficulty.
  //! See also 
  //! https://bitcointalk.org/index.php?topic=23266.0
  //!
  //! Computes min difficulty allowed based on
  //! the last reliable block difficulty.
  //!
  //! @author Gavin Andresen <gavinandresen@gmail.com>
  //! @author Sergei Lodyagin <serg@kogorta.dp.ua>
  template<class Block>
  compact_bignum_t difficulty::dos_min_difficulty(
    const Block& block
  ) const
  {
    const auto* last_reliable_block =
      dos_last_reliable_block();

    const auto past = block.GetTimePoint() 
      - last_reliable_block->GetTimePoint();
  
    return std::min(
      CBigNum(last_reliable_block->nBits)
        * past / block_period_by_design
        * adjustment_by_design,
      CBigNum(min_difficulty_by_design)
    ).GetCompact();
  }


} // DigiByte
