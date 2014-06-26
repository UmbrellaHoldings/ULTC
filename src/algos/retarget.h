// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * A retarget algo abstraction
 *
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_RETARGET_H
#define BITCOIN_RETARGET_H

#include <algorithm>
#include "types.h"
#include "bignum.h"
#include "btc_time.h"
#include "pars.h"
#include "block.h"

namespace retarget {

// An abstract difficulty singleton
class difficulty
{
public:
  using duration = coin::times::block::clock::duration;

  static difficulty& instance();

  //! Calculates a target difficulty based on the actual
  //! timespan value
  virtual compact_bignum_t next_block_difficulty(
    duration desired_timespan,
    duration actual_timespan,
    compact_bignum_t last_block_difficulty
  ) = 0;

  //! Do not accept blocks with too low difficulty.
  //! See also 
  //! https://bitcointalk.org/index.php?topic=23266.0
  //!
  //! Computes min difficulty allowed based on
  //! the last reliable block difficulty and time past.
  //! @exception coin::except::invalid_timestamp
  //!
  //! @author Gavin Andresen <gavinandresen@gmail.com>
  //! @author Sergei Lodyagin <serg@kogorta.dp.ua>
  virtual compact_bignum_t dos_min_difficulty(
    compact_bignum_t last_reliable_block_difficulty,
    coin::times::block::clock::duration past
  ) const = 0;

  //! Do not accept blocks with too low difficulty.
  //! See also 
  //! https://bitcointalk.org/index.php?topic=23266.0
  //!
  //! Computes min difficulty allowed based on
  //! the last reliable block difficulty.
  //! @exception coin::except::invalid_timestamp
  //!
  //! @author Gavin Andresen <gavinandresen@gmail.com>
  //! @author Sergei Lodyagin <serg@kogorta.dp.ua>
  template<class Block>
  compact_bignum_t dos_min_difficulty(
    const Block& block
  ) const
  {
    const auto* last_reliable_block =
      this->dos_last_reliable_block();

    const auto past = block.GetTimePoint() 
      - last_reliable_block->GetTimePoint();

    return dos_min_difficulty(
      last_reliable_block->nBits,
      past
    );
  }

  //! Calculates a target difficulty for the next block
  compact_bignum_t next_block_difficulty(
    const CBlockIndex* pindexLast
  );

  //! Checks the difficulty is in implementation defined
  //! range.
  // TODO ComputeMinWork?
  template<class Block>
  bool is_valid(const Block& block)
  {
    const auto diff = block.nBits;

    return diff > 0 && 
      diff <= min_difficulty_by_design;
  }

  //! Call dos_min_difficulty() and throw exception if it
  //! is lower for the block.
  //! @exception coin::except::invalid_difficulty
  //! @exception coin::except::invalid_timestamp
  template<class Block>
  void dos_check_min_difficulty(const Block& block) const
  {
    if (block.nBits.lower_difficulty
        (dos_min_difficulty(block))
        )
      throw types::exception
        <coin::except::invalid_difficulty>
      (
          "block with too little proof-of-work"
      );
  }

  //! The greater numeric value is the lower difficulty
  const compact_bignum_t min_difficulty_by_design;

  //! An average planned block period
  const duration block_period_by_design;

protected:
  difficulty();

  //! Returns the last block which can't be faked
  static const CBlockIndex* dos_last_reliable_block();
};

} // retarget

#endif
