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
namespace umbrella { block* create(); }

}

namespace pars {

// hash function

enum class hash_fun
{
  sha256d,
  scrypt,
  brittcoin_scrypt
};

constexpr auto hash_function = hash_fun::sha256d;

// mining profile

/** No amount larger than this (in satoshi) is valid */
constexpr int64 MAX_MONEY = 35000000 * COIN;

// genesis block

inline genesis::block* create_genesis_block()
{
  return genesis::umbrella::create();
}

// retarget algo

enum class retarget_algo
{
  twice_and_half, // for test only
  digishield,
  kgw
};

constexpr auto retarget_algorithm = 
  retarget_algo::twice_and_half;
//  retarget_algo::digishield;

// the value second is for testnet
constexpr auto min_difficulty_by_design = std::make_pair(
//  0x1d0fffff, 0x1d7fffff
  0x207fffff, 0x207fffff
);

// the value second is for testnet
constexpr auto block_period_by_design = std::make_pair(
  coin::times::block::seconds(30),
  coin::times::block::seconds(30)
);

template<class T>
T testnet_switch(const std::pair<T, T>& par)
{
  const bool is_testnet = GetBoolArg("-testnet");
  return (is_testnet) ? par.second : par.first;
}

} // pars

constexpr int64 MAX_MONEY = pars::MAX_MONEY;

inline bool MoneyRange(int64 nValue) 
{ 
  return (nValue >= 0 && nValue <= MAX_MONEY); 
}

#endif

