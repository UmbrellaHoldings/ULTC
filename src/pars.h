// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The altcoin parameters. Change it to get various
 * altcoins.
 *
 * @author Sergei Lodyagin
 */

#ifndef BITCOIN_PARS_H
#define BITCOIN_PARS_H

#include <utility>
#include "util.h"
#include "block.h"
#include "btc_time.h"

namespace genesis {

class block;

namespace bitcoin  { block* create(); }

namespace litecoin { 
  block* create(); 
  block* create_testnet(); 
}

namespace umbrella { 
  block* create(); 
  block* create_testnet(); 
}

}

namespace pars {

template<class T>
T testnet_switch(const std::pair<T, T>& par)
{
  const bool is_testnet = GetBoolArg("-testnet");
  return (is_testnet) ? par.second : par.first;
}

// hash function

enum class hash_fun
{
  sha256d,
  scrypt,
  brittcoin_scrypt
};

constexpr auto hash_function = hash_fun::scrypt;

// mining profile

/** No amount larger than this (in satoshi) is valid */
constexpr int64 MAX_MONEY = 6000000 * COIN;

// genesis block

inline genesis::block* create_genesis_block()
{
  return testnet_switch(
    std::make_pair(
      genesis::umbrella::create,
      genesis::umbrella::create_testnet
    )
  )();
}

// the value second is for testnet
constexpr auto block_period_by_design = std::make_pair(
  coin::times::block::seconds(270),
  coin::times::block::seconds(27)
);

// retarget algo

struct retarget_algo {};
struct twice_and_half : retarget_algo {}; // for test only
struct digishield : retarget_algo {};

struct kgw : retarget_algo
{
  constexpr static coin::times::block::clock::duration 
  past_min()
  {
    return coin::times::block::hours(1);
  }

  constexpr static coin::times::block::clock::duration 
  past_max()
  {
    return coin::times::block::days(1);
  }

};

static_assert(
  kgw::past_min() > block_period_by_design.first
  && kgw::past_min() > block_period_by_design.second,
  "invalid past_min() definition"
);

static_assert(
  kgw::past_max() > block_period_by_design.first
  || kgw::past_max() > block_period_by_design.second
  || kgw::past_max() > kgw::past_min() 
       + std::max(
           block_period_by_design.first,
           block_period_by_design.second
         ),
  "invalid past_max() definition"
);

using retarget_algorithm = digishield;

// the value second is for testnet
constexpr auto min_difficulty_by_design = std::make_pair(
//  0x1d0fffff, 0x1d7fffff
  0x207fffff, 0x207fffff
);

} // pars

constexpr int64 MAX_MONEY = pars::MAX_MONEY;

inline bool MoneyRange(int64 nValue) 
{ 
  return (nValue >= 0 && nValue <= MAX_MONEY); 
}

#endif

