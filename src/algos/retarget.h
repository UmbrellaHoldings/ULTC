// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
//////////////////////////////////////////////////////////

/**
 * @file
 * A retarget algo abstraction
 *
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_RETARGET_H
#define BITCOIN_RETARGET_H

#include <algorithm>
#include "types/abstract_iterator.h"
#include "types.h"
#include "bignum.h"
#include "btc_time.h"
#include "pars.h"
#include "block.h"

namespace block {

struct info_type 
{
  int height;
  compact_bignum_t difficulty;
  coin::times::block::clock::time_point time;
};

template<class Block>
class const_iterator 
  : public types::virtual_iterator::const_forward_base<
      info_type
    >
{
  using base = types::virtual_iterator::const_forward_base<
    info_type
  >;

public:
  using value_type = typename base::value_type;
  using reference = typename base::reference;
  using pointer = typename base::pointer;
  using iterator_category = 
    typename base::iterator_category;
  using difference_type = typename base::difference_type;
 
  const_iterator(const Block* blk_ = nullptr) : blk(blk_)
  {}

  reference operator*() const override
  {  
    using namespace coin::times::block;

    if (!blk)
     throw types::exception
      <types::virtual_iterator::dereference_of_unitialized>
        ();

    return value_type{
      blk->nHeight, 
      blk->nBits, 
      clock::from_nTime(blk->nTime)
    };
  }

  base& operator++() override
  {
    if (!blk)
      throw types::exception
        <types::virtual_iterator::movement_of_unitialized>
          ();

    blk = blk->pprev;
    return *this;
  }

  std::unique_ptr<base> clone() const override
  {
    return std::unique_ptr<base>(new const_iterator(blk));
  }

  bool operator==(const base& o) const override
  {
    auto* other = dynamic_cast<const const_iterator*>(&o);

    if (!other)
      throw types::exception<
        types::virtual_iterator::incompatible_types
      > ();

    return blk == other->blk;
  }

protected:
  const Block* blk;
};

} // block

namespace retarget {

// An abstract difficulty singleton
class difficulty
{
public:
  using duration = coin::times::block::clock::duration;
  using time_point = coin::times::block::clock::time_point;

  using iterator =
    types::virtual_iterator::const_forward_holder<
      block::info_type
    >;

  static iterator::difference_type 
  height_diff(
    const iterator& a,
    const iterator& b
  )
  {
    return a->height - b->height;
  }

  static difficulty& instance();

  //! Calculates a target difficulty based on the actual
  //! timespan value
  virtual compact_bignum_t next_block_difficulty(
    const duration desired_timespan,
    //! the last block in chain
    const iterator& rbegin,
    //! the first block in chain
    const iterator& rend
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
