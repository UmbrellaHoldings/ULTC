// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * A bitcoin block.
 *
 * @author Satoshi Nakamoto
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_BLOCK_H
#define BITCOIN_BLOCK_H

#include <exception>

namespace coin {

namespace except {

//! @exception Invalid block detected
struct invalid_block : virtual std::exception {};

//! @exception DoS attack detected
struct dos : virtual std::exception {};

//! @exception The block timestamp is invalid
struct invalid_timestamp : invalid_block, dos {};

//! @exception The block difficulty is invalid
struct invalid_difficulty : invalid_block, dos {};

}

}

#endif

