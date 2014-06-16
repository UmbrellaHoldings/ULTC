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
#include "types.h"
#include "bignum.h"

class CBlockIndex;

namespace DigiByte {

// TODO singleton
class difficulty
{
public:
  using duration = coin::time::block::clock::duration;

  //! Calculates a target difficulty for the next block
  compact_bignum_t next_block_difficulty(
    const CBlockIndex* pindexLast
  );

  //! Checks the difficulty is in implementation defined
  //! range.
  // TODO ComputeMinWork?
  template<class Block>
  bool is_valid(const Block& block);

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
  ) const;

  //! Call dos_min_difficulty() and throw exception if it
  //! is lower for the block.
  //! @exception coin::except::invalid_difficulty
  //! @exception coin::except::invalid_timestamp
  template<class Block>
  void dos_check_min_difficulty(const Block& block) const;

  //! An average planned block period
  const duration block_period_by_design = 
    coin::time::block::minutes(8);

protected:
  //TODO make the _by_design parameters as a template
  //parameters 

  //! The greater numeric value is the lower difficulty
  const compact_bignum_t min_difficulty_by_design = 
    CBigNum(~uint256(0) >> 18).GetCompact();

  //! The limit parameter for dos_min_difficulty()
  const coin::percent_t adjustment_by_design = 
    coin::operator"" _pct(110);

  //! Returns the last block which can't be faked
  static const CBlockIndex* dos_last_reliable_block();
};

} // DigiByte

#endif
