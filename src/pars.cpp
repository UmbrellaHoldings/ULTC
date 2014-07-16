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

genesis::block* create_genesis_block()
{
  return testnet_switch(
    types::make_pair(
      genesis::umbrella::create,
      genesis::umbrella::create_testnet
    )
  )();
}

} // pars
