// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Basic types
 *
 * @author Sergei Lodyagin
 */

#ifndef BITCOIN_TYPES_H
#define BITCOIN_TYPES_H

#include <cstdint>
#include "types/fixed.h"
#include "types/exception.h"

namespace coin {

using percent_t = types::percent_t<intmax_t>;

inline percent_t operator""_pct(long double p)
{
  bool lost;
  const auto res = percent_t::from_long_double(p, &lost);

  if (lost)
    throw types::exception<types::precision_lost>(
      "Unable to represent ", p, 
      "_pct as a fixed point type coin::percent_t"
    );

  return res;
}

}

#endif
