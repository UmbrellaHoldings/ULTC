// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Basic types
 *
 * @author Sergei Lodyagin
 */

#ifndef BITCOIN_TYPES_H
#define BITCOIN_TYPES_H

#include <exception>
#include <cstdint>
#include "types/fixed.h"

namespace types {

struct invalid_literal : virtual std::exception {};

struct precision_lost_in_literal
  : invalid_literal, precision_lost
{};

//! Use it for constexpr until c++14
template<class T1, class T2> struct pair
{
  using first_type = T1;
  using second_type = T2;

  T1 first;
  T2 second;

  constexpr pair() {}

  constexpr pair(const T1& x, const T2& y)
    : first(x), second(y)
  {}

#if 0 // needs constexpr forward
  template<class U1, class U2>
  constexpr pair(U1&& x, U2&& y) 
    : first(types::forward<U1>(x)),
      second(types::forward<U2>(y))
  {}
#endif
};

//! TODO no special case for reference_wrapper
#if 0 // needs constexpr forward
template< class T1, class T2 >
constexpr auto make_pair(T1&& t, T2&& u)
 -> pair<
      typename std::decay<T1>::type, 
      typename std::decay<T2>::type
    >
{
  return pair<
    typename std::decay<T1>::type, 
    typename std::decay<T2>::type
  >(
    std::forward<T1>(t),
    std::forward<T2>(u)
  );
}
#else
template< class T1, class T2 >
constexpr pair<T1, T2> make_pair(T1 t, T2 u)
{
  return pair<T1, T2>(t, u);
}
#endif



} // types

using percent_t = 
types::fixed_t<intmax_t, std::ratio<1, 100>>;

percent_t operator"" _pct(long double p);

#endif
