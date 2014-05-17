// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Basic types
 *
 * @author Sergei Lodyagin
 */

#include "types.h"
#include "types/exception.h"

//namespace types {
//class template fixed_t<intmax_t, std::ratio<1, 100>>;
//}

namespace coin {

percent_t operator"" _pct(long double p)
{
  bool lost;
  const auto res = 
    percent_t::from_long_double(p, &lost) / 100;

  if (lost)
    throw types::exception<types::precision_lost>(
      "Unable to represent ", p, 
      "_pct as a fixed point type coin::percent_t"
      " (lost precision)"
    );

  return res;
}

}
