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

#include "util.h"
#include "block.h"

namespace genesis {

class block;

namespace bitcoin { block* create(); }

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
  return genesis::bitcoin::create();
}

} // pars

constexpr int64 MAX_MONEY = pars::MAX_MONEY;

inline bool MoneyRange(int64 nValue) 
{ 
  return (nValue >= 0 && nValue <= MAX_MONEY); 
}

#endif

