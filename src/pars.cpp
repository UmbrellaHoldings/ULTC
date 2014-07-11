// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The altcoin parameters. Change it to get various
 * altcoins.
 *
 * @author Sergei Lodyagin
 */

#include "pars.h"

namespace hash {
namespace scrypt {
namespace brittcoin {

scratchpad_ptr get_scratchpad(n_factor_t n_factor);

uint256 hash(
  const CBlock& blk,
  n_factor_t n_factor,
  scratchpad_ptr scr
);

} // brittcoin
} // scrypt
} // hash

namespace pars {

hash::scratchpad_ptr n_factor_scrypt::get_scratchpad(
  n_factor_t n_factor
)
{
  return hash::scrypt::brittcoin::get_scratchpad(n_factor);
}

uint256 n_factor_scrypt::hash(
  const CBlock& blk,
  n_factor_t n_factor,
  hash::scratchpad_ptr scr
)
{
  return hash::scrypt::brittcoin::hash(blk, n_factor, scr);
}

money_t mining_profile::max_money()
{
  return 1000000000_coin;
}

money_t mining_profile::premined_money()
{
  return block_reward();
}

money_t mining_profile::block_reward()
{
  return 1_coin;
}

money_t mining_profile::GetBlockValue(
  int nHeight, 
  money_t nFees
)
{
  auto nSubsidy = block_reward();
  
  if(nHeight == 1)
    nSubsidy = premined_money();
  else if (nHeight > max_money_height())
    nSubsidy = money_t::zero();

  return nSubsidy + nFees;
}

genesis::block* create_genesis_block()
{
#ifdef TECH
//  tech::testmaxmoney();
#endif
  return testnet_switch(
    std::make_pair(
      genesis::umbrella::test_mm::create,
      genesis::umbrella::test_mm::create_testnet
    )
  )();
}

//! merged mining pars
namespace mm {

// to enable merged mining:
// - set a block from which it will be enabled
// - set a unique chain ID
//   each merged minable scrypt_1024_1_1_256 coin
//   should have a different one
//   (if two have the same ID, they can't be 
//   merge mined together)
int GetOurChainID()
{
  return 0x0000;
}

} // mm

} // pars
