// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The altcoin parameters. Change it to get various
 * altcoins.
 *
 * @author Sergei Lodyagin
 */

#include "pars.h"

namespace pars {

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
  return 0x0001;
}

} // mm

} // pars
