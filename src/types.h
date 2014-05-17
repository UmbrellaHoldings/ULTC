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

namespace coin {

using percent_t = types::percent_t<intmax_t>;

percent_t operator""_pct(long double p);

}

#endif
