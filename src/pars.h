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
#include "key.h"

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

} // genesis

namespace hash {

struct scratchpad_base;
using scratchpad_ptr = scratchpad_base*;

} // hash

//! Chain type for merged mining
//enum mm_chain { parent, aux };

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
constexpr int64 PREMINED_MONEY = 120000 * COIN;
constexpr int64 BLOCK_REWARD = 10 * COIN;

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
  coin::times::block::seconds(270),//for live
  coin::times::block::seconds(27)  //for testnet
);

// retarget algo

struct retarget_algo {};
struct twice_and_half : retarget_algo {}; // for test only
struct digishield : retarget_algo 
{
  constexpr static bool limit_steps()
  {
    return false;
  }
};

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

using retarget_algorithm = kgw;

// the value second is for testnet
constexpr auto min_difficulty_by_design = std::make_pair(
  0x1e07ffff, 0x1e6236f6
);

namespace coinbase {

const CPubKey reward_collecting_pubkey(
  ParseHex("038fd4af3230e85df2ba018c57827df8ab04ed92ceff536bbec3aee6a51a563924")
);

} // coinbase

//! merged mining pars
namespace mm {

// to enable merged mining:
// - set a unique chain ID
//   each merged minable scrypt_1024_1_1_256 coin
//   should have a different one
//   (if two have the same ID, they can't be 
//   merge mined together)
constexpr auto chain_id = std::make_pair(1, 2);

} // mm

} // pars

constexpr int64 MAX_MONEY = pars::MAX_MONEY;

inline bool MoneyRange(int64 nValue) 
{ 
  return (nValue >= 0 && nValue <= pars::MAX_MONEY); 
}

#endif

