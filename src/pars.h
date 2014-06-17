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

namespace pars {

enum class hash_fun
{
  sha256d,
  scrypt,
  brittcoin_scrypt
};

constexpr auto hash_function = hash_fun::brittcoin_scrypt;

} // pars

#endif

