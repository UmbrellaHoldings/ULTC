// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Calculates the "N-factor", it is yacoin/vertcoin concept
 * originally, see 
 * http://www.followthecoin.com/interview-creator-vertcoin/
 *
 * @author Sergei Lodyagin
 */

#ifndef BITCOIN_N_FACTOR_H
#define BITCOIN_N_FACTOR_H

#include <tuple>
#include "btc_time.h"

using n_factor_t = std::tuple<uint32_t, unsigned, unsigned>;

n_factor_t
GetNfactor(coin::times::block::time_point block_time);

#endif
