// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * Arithmetic operations with overflow control
 *
 * @author    Sergei Lodyagin
 * @copyright Copyright (C) 2013 Sergei Lodyagin 
 */

#ifndef TYPES_SAFE_H
#define TYPES_SAFE_H

#include <iostream>
#include <type_traits>
#include <stdexcept>
#include <limits>

namespace types {

//namespace bits_ {

template<class UInt, class = void>
struct bits;

template<>
struct bits<unsigned int>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(unsigned i)
  {
    return i
      ? sizeof(i) * 8 - __builtin_clz(i)
      : 0;
  }
};

template<>
struct bits<unsigned long>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(unsigned long i)
  {
    return i
      ? sizeof(i) * 8 - __builtin_clzl(i)
      : 0;
  }
};

template<>
struct bits<unsigned long long>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(unsigned long long i)
  {
    return i
      ? sizeof(i) * 8 - __builtin_clzll(i)
      : 0;
  }
};

#if 0
template<>
struct bits<
  UInt,
  typename std::enable_if<
    std::is_integral<UInt>::value, 
    // have no sence for signed, and so
    // disabled to catch logic error in your program
    std::is_unsigned<UInt>::value
  >::type
>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(UInt i)
  {
  }
}
#endif

//} // bits_

template <class UInt>
unsigned highest_bit1(UInt i)
{
#if 1 // using GCC builtin
  return bits<UInt>::highest_1(i);
#else
  int res;
  // TODO check with assembly
  for (res = 0; i != 0; i >>= 1, ++res)
    ;
  return res;
#endif
}

//! Not defined for not integral and unsigned integral types
template <
  class Int, 
  bool = std::is_integral<Int>::value,
  bool = std::is_signed<Int>::value
>
class safe;

/**
  * It just an integer overflow checking wrapper for any
  * integral type. It accumulates the overflow in a flag to
  * appear later. There are two basic places where it will
  * appear: operator bool() which return the overflow as
  * true (no overflow) / false and cast to number which
  * will raise std::overflow_error. Usually you need to
  * check a result for overflow before casting this class
  * to number.
  *
  * Implementing ideas from 
  * http://www.fefe.de/intof.html and
  * https://www.securecoding.cert.org/confluence/display/seccode/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
  *
  *
  * @autor Sergei Lodyagin
  */
template<class Int>
class safe<Int, true, true>
{
  static_assert( std::is_signed<Int>::value, 
                 "modular_type must be signed" );

  template<class Int2, bool, bool>
  friend class safe;
public:
  static constexpr Int max = 
    std::numeric_limits<Int>::max();

  static constexpr Int min = 
    std::numeric_limits<Int>::min();

  //! The default value is overflow
  constexpr safe() noexcept : no_ovf(false) {}

  explicit constexpr safe(Int av) noexcept
    : safe(av, true) {}

  safe& operator = (Int av)
  {
    v = av;
    no_ovf = true;
    rem = false;
    return *this;
  }

  /*
   * There is a solution based on a highest bits
   * calculation but it works for unsigned only. Checking
   * sign of argument is expensive because we will have
   * no jump prediction for such condition.
   */
  safe& operator += (safe sb) noexcept
  {
    Int& a = v;
    const Int& b = sb.v;

    // NB & not &&
    if (__builtin_expect((b > 0) & (max - b < a), 0)) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect((b < 0) & (min - b > a), 0)) {
      no_ovf = false;
      return *this;
    }

    a += b;
    inherit_status(sb);
    return *this;
  }

  safe& operator += (Int b) noexcept
  {
    return operator+=(safe(b));
  }

  safe& operator -= (safe b) noexcept
  {
    return operator+=(-b);
  }

  safe& operator -= (Int b) noexcept
  {
    return operator-=(safe(b));
  }

  template<class Int2>
  safe& operator *= (safe<Int2> b) noexcept
  {
    // TODO check what is faster
#if 1
    const typename std::make_unsigned<Int>::type ua = 
      std::abs(v);
    const typename std::make_unsigned<Int2>::type ub = 
      std::abs(b.v);

    if (__builtin_expect(
          highest_bit1(ua) + highest_bit1(ub) > 
          sizeof(Int)*8/2-1, 
          0
        ))
      no_ovf = false;
    else {
      v *= b.v;
      inherit_status(b);
    }
#else
    Int& a = v;
    const Int& b = sb.v;

    // NB & not &&
    if (__builtin_expect
         ((a > 0) & (b > 0) & (a > max / b), 0) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect
         ((a > 0) & (b <= 0) & (b < min / a), 0) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect
         ((a <= 0) & (b > 0) & (a < min / b), 0) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect
         ((a < 0) & (b <= 0) & (b < max / a), 0) {
      no_ovf = false;
      return *this;
    }
    
    a *= b;
    inherit_status(sb);
#endif

    return *this;
  }

  template<class Int2>
  safe& operator *= (Int2 b) noexcept
  {
    return operator*=(safe<Int2>(b));
  }

  template<class Int2>
  safe& operator /= (safe<Int2> b) noexcept
  {
    Int copy = v;

    operator%=(b);
    // division overflow and overflow in b are alredy
    // checked in operator %=
  
    rem = rem || v != 0;
    v = copy; // NB no_ovf is the same
    
    return *this;
  }

  safe& operator /= (Int b) noexcept
  {
    return operator/=(safe(b));
  }

  template<class Int2>
  safe& operator %= (safe<Int2> b) noexcept
  {
    // it seams except for b == 0 this operation is always
    // safe.

    // division by zero is overflow
    if (__builtin_expect(b.v == 0, 0)) {
      no_ovf = false;
      return *this;
    }
    // is it needed? If no, move to /=
    if (__builtin_expect(v == min, 0)
        && __builtin_expect(b.v == -1, 0)) 
    {
      no_ovf = false;
      return *this;
    }
    

    v %= b.v;
    inherit_status(b);
    return *this;
  }

  safe& operator %= (Int b) noexcept
  {
    return operator%=(safe(b));
  }

  safe operator + () const noexcept
  {
    return *this;
  }

  safe operator - () const noexcept
  {
    using namespace std;
    return safe
      (v, (abs(min) <= abs(max) || v != min)
       && (abs(max) <= abs(min) || v != max));
  }

  safe operator + (safe b) const noexcept
  {
    return b += *this;
  }

  safe operator - (safe b) const noexcept
  {
    return b -= *this;
  }

  safe operator * (safe b) const noexcept
  {
    return b *= *this;
  }

  template<class Int2>
  safe operator * (safe<Int2> b) const noexcept
  {
    safe copy(*this);
    return copy *= b;
  }

  template<class Int2>
  safe operator / (safe<Int2> b) const noexcept
  {
    safe<Int> copy(*this);
    return copy /= b;
  }

  safe operator % (safe b) const noexcept
  {
    safe<Int> copy(*this);
    return copy %= b;
  }

  bool operator == (safe b) const noexcept
  {
    return v == b.v;
  }

  bool operator != (safe b) const
  {
    throw_overflow();
    return v != b.v;
  }

  bool operator < (safe b) const
  {
    throw_overflow();
    return v < b.v;
  }

  bool operator <= (safe b) const
  {
    throw_overflow();
    return v <= b.v;
  }

  bool operator > (safe b) const
  {
    throw_overflow();
    return v > b.v;
  }

  bool operator >= (safe b) const
  {
    throw_overflow();
    return v >= b.v;
  }

  explicit operator Int () const
  {
    throw_overflow();
    return v;
  }

  explicit operator long double () const
  {
    return (Int) *this;
  }

  explicit constexpr operator bool () const
  {
    return no_ovf;
  }

  static constexpr safe overflow() 
  { 
    return safe(); 
  }

  constexpr bool lost_precision() const
  {
    return rem;
  }

  //! Set the lost_precision flag if the argument is true.
  //! \return the rem flag
  bool lost_precision(bool lp) noexcept
  {
    return rem = rem || lp;
  }

  static const std::exception& overflow_exception()
  {
    static std::overflow_error
      exc("class safe: unchecked overflow");
    return exc;
  }

protected:
  Int v;

  // TODO what is better?
#if 0
  int8_t 
    no_ovf : 1, 
    rem : 1 = false;
#else
  bool no_ovf;
  //! 
  bool rem = false;
#endif

  constexpr safe(Int val, bool no) noexcept
    : v(val), no_ovf(no) {}

  //! Inherit status bits, typically from the second
  //! operand of an binary function (e.g. operator+)
  template<class Int2>
  void inherit_status(const safe<Int2>& other)
  {
    no_ovf = no_ovf && other.no_ovf;
    rem = rem || other.rem;
  }

  void throw_overflow() const
  {
    if (__builtin_expect(!no_ovf, 0))
      throw overflow_exception;
  }
};

template<class I>
safe<I> operator + (safe<I> a, I b) noexcept
{
  return a + safe<I>(b);
}

template<class I>
safe<I> operator + (I a, safe<I> b) noexcept
{
  return safe<I>(a) + b;
}

template<class I>
safe<I> operator - (safe<I> a, I b) noexcept
{
  return a - safe<I>(b);
}

template<class I>
safe<I> operator - (I a, safe<I> b) noexcept
{
  return safe<I>(a) - b;
}

template<class I>
safe<I> operator * (safe<I> a, I b) noexcept
{
  return a * safe<I>(b);
}

template<class I>
safe<I> operator * (I a, safe<I> b) noexcept
{
  return safe<I>(a) * b;
}

template<class I>
safe<I> operator / (safe<I> a, I b) noexcept
{
  return a / safe<I>(b);
}

template<class I>
safe<I> operator / (I a, safe<I> b) noexcept
{
  return safe<I>(a) / b;
}

template<class I>
safe<I> operator % (safe<I> a, I b) noexcept
{
  return a % safe<I>(b);
}

template<class I>
safe<I> operator % (I a, safe<I> b) noexcept
{
  return safe<I>(a) % b;
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>&
operator << 
  ( std::basic_ostream<CharT, Traits>& out,
    const safe<Int>& s )
{
  return (bool) s ? out << (Int) s : out << "#overflow";
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>&
operator << 
  ( std::basic_ostream<CharT, Traits>&& out,
    const safe<Int>& s )
{
  return operator <<(out, s);
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_istream<CharT, Traits>&
operator >>
  ( std::basic_istream<CharT, Traits>& in, safe<Int>& s )
{
  Int val;
  typename std::basic_istream<CharT, Traits>::sentry se(in);

  if (!se)
    return in;

  in >> val;
  if (!in.fail())
    s = safe<Int>(val);
  else
    s = safe<Int>::overflow();
  return in;
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_istream<CharT, Traits>&
operator >> 
  ( std::basic_istream<CharT, Traits>&& in, safe<Int>& s )
{
  return operator >> (in, s);
}

} // types

#endif
