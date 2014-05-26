// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Satoshi Nakamoto
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#include "bignum.h"

std::ostream&
operator<<(std::ostream& out, compact_bignum_t c)
{
  using namespace std;
  out << setfill('0') << hex << showbase
      << setw(sizeof(c.compact)*8/4)
      << c.compact << "_comp" << dec << noshowbase;
  return out;
}

