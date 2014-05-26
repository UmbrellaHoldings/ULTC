// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Anastasia Kurbatova
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_FIXED_H_
#define COHORS_TYPES_FIXED_H_

#include <ostream>
#include <iomanip>
#include <string>
#include <utility>
#include <type_traits>
#include <ratio>
#include <chrono>
#include <locale>
#include <exception>
#include "types/safe.h"
#include "types/exception.h"

namespace types {

//! Unable represent the number 
//! with the actual fixed_t type.
//! E.g., it can be used in operator"".
struct precision_lost : virtual std::exception {};

/**
 * A fixed point type.
 *
 * @author Anastasia Kurbatova
 * @author Sergei Lodyagin
 */
template<class Rep, class Ratio>
class fixed_t
{
  template<class Re, class Ra>
  friend fixed_t operator*(Rep a, fixed_t b) noexcept;

  template<class Int, class Re, class Ra>
  friend fixed_t operator*
    (const types::safe<Int>& a, fixed_t b) noexcept;

  template<class Re, class Ra>
  friend fixed_t operator/(fixed_t a, fixed_t b) noexcept;

  template <class, class, class, class>
  friend class fixed_put;

//  friend fixed_t operator "" _fx (long double p);

public:
  typedef Rep modular_type;
  typedef Ratio ratio_type;

  static constexpr auto num = ratio_type::num;
  static constexpr auto den = ratio_type::den;

#if 0
  static constexpr double ratio_as_long_double = 
    (double) ratio_type::num / ratio_type::den;
  static constexpr double rev_ratio_as_long_double = 
    (double) ratio_type::den / ratio_type::num;
#endif

  static_assert( 
    std::is_signed<modular_type>::value, 
    "modular_type must be signed. "
    "Using unsigned for overflow-controlled arithmetic is "
    "impossible because we don't know what is "
    "(unsigned) -1 really."
  );

  static_assert(
    ratio_type::den != 0,
    "fixed_t::ratio_type::den == 0"
  );

  static_assert(
    ratio_type::num != 0,
    "fixed_t::ratio_type::num == 0"
  );

  static_assert(
    ratio_type::num == 1,
    "fixed_t::ratio_type::num != 1"
  );

  constexpr fixed_t() : rep(0) {}

  constexpr fixed_t(std::chrono::duration<Rep, Ratio> d)
    : rep(d.count())
  {}

  //! The maximal fixed_t value
  static constexpr fixed_t max() 
  {
    return 
      fixed_t(std::numeric_limits<modular_type>::max());
  }

  //! The minimal fixed_t value
  static constexpr fixed_t min() 
  {
    return 
      fixed_t(std::numeric_limits<modular_type>::min());
  }

  static_assert(
    ratio_type::num == 1, 
    "fixed_t::one() is undefined"
  );
  static constexpr fixed_t one()
  {
    return fixed_t(ratio_type::den);
  }

  static constexpr fixed_t zero()
  {
    return fixed_t(0);
  }

  //! Returns the smallest absolute value.
  static constexpr fixed_t bit()
  {
    return fixed_t(1);
  }

  //! Convert to long double.
  //! Can loss precision. If lost_precision != nullptr set
  //! *lost_precision = true if either this operation loss
  //! precision or the loss precision flag is already set
  //! in the rep.
  //! \throws std::overflow_exception an overflow in the
  //! past 
  void to_long_double
    (long double& val, bool* lost_precision) const
  {
    val = (long double) rep / num; //* ratio_as_long_double;
    if (lost_precision) // a recursion guard
      *lost_precision = 
        *this != from_long_double(val, nullptr)
        || rep.lost_precision (); 
           // as a result of division
  }

  //! Construct fixed_t from a long double value.
  //! If loss precision set the lost_precision
  //! flag in this fixed_t value and also return in
  //! *loss_precision. If loss_precision == nullptr never
  //! set the flag in the fixed_t value.
  static fixed_t from_long_double
    (long double d, bool* lost_precision) 
  {
    fixed_t p(d * num/*rev_ratio_as_long_double*/);
    if (lost_precision) { // a recursion guard
      long double d2;
      p.to_long_double(d2, nullptr);
      *lost_precision = p.rep.lost_precision(d != d2);
    }
    return p;
  }

  fixed_t operator - () const noexcept
  { 
    return fixed_t(rep); 
  }

  bool operator < (fixed_t p) const
  {
    return rep < p.rep;
  }

  bool operator > (fixed_t p) const noexcept
  {
    return rep > p.rep;
  }
  bool operator <= (fixed_t p) const noexcept
  {
    return !operator>(p);
  }

  bool operator >= (fixed_t p) const noexcept
  {
    return !operator<(p);
  }

  bool operator == (fixed_t p) const noexcept
  {
   return rep == p.rep;
  }

  bool operator!=(fixed_t p) const noexcept
  {
    return !operator==(p);
  }

  fixed_t& operator += (fixed_t p) noexcept
  {
    rep += p.rep;
    return *this;
  }

  fixed_t& operator -= (fixed_t p) noexcept
  {
    rep -= p.rep;
    return *this;
  }

  fixed_t& operator *= (modular_type p) noexcept
  {
    rep *= p;
    return *this;
  }

  template<class Int>
  fixed_t& operator *= 
    (const types::safe<Int>& p) noexcept
  {
    rep *= p;
    return *this;
  }

  fixed_t& operator /= (modular_type p) noexcept
  {
    rep /= p;
    return *this;
  }

  template<class Int>
  fixed_t& operator /=
    (const types::safe<Int>& p) noexcept
  {
    rep /= p;
    return *this;
  }

  fixed_t& operator/=(fixed_t o) noexcept
  {
    rep /= o.rep;
    return *this;
  }

  fixed_t operator + (fixed_t p) const noexcept
  {
    return fixed_t(rep + p.rep);
  }

  fixed_t operator - (fixed_t p) const noexcept
  {
    return fixed_t(rep - p.rep);
  }
  
  fixed_t operator * (modular_type p) const noexcept
  {
    return fixed_t(rep * p);
  }

  template<class Int>
  fixed_t operator * (const types::safe<Int>& p) const 
    noexcept
  {
    return fixed_t(rep * p);
  }

  //! Divizion with truncation.
  //! Division by zero is stored as an overflow
  fixed_t operator / (modular_type p) const noexcept
  {
    return fixed_t(rep / p);
  }

  //! Divizion with truncation.
  //! Division by zero is stored as an overflow
  template<class Int>
  fixed_t operator / 
    (const types::safe<Int>& p) const noexcept
  {
    return fixed_t(rep / p);
  }

  fixed_t operator % (fixed_t b) const noexcept
  {
    // Alex Stepanov's explains the proper sign
    // of % operation, C++-11 do according to it.
    return fixed_t(rep % b.rep);
  }

  //! Check an overflow occurence in the past
  explicit operator bool() const noexcept
  {
    return (bool) rep;
  }

  //! Truncates to an integer
  safe<modular_type> truncate() const noexcept
  {
    return rep * ratio_type::num / ratio_type::den;
  }

protected:
  safe<modular_type> rep;

  // TODO make protected
  explicit constexpr fixed_t
    (const types::safe<modular_type>& arep) 
    : rep(arep) {}  

  explicit constexpr fixed_t(modular_type arep)
    : fixed_t(types::safe<modular_type>(arep)) {}
};

template<class Re, class Ra>
fixed_t<Re, Ra> operator*(
  typename fixed_t<Re, Ra>::modular_type a, 
  fixed_t<Re, Ra> b
) noexcept
{
  return b.operator*(a);
}

template<class Int, class Re, class Ra>
fixed_t<Re, Ra> operator*( 
  const types::safe<Int>& a, 
  fixed_t<Re, Ra> b 
) noexcept
{
  return b.operator*(a);
}

template<class Re, class Ra>
fixed_t<Re,Ra> operator/(
  fixed_t<Re,Ra> a, 
  fixed_t<Re,Ra> b
) noexcept
{
  return a /= b;
}

template<class Rep, class Ratio>
constexpr fixed_t<Rep, Ratio> 
//
to_fixed(std::chrono::duration<Rep, Ratio> dur)
{
  return fixed_t<Rep, Ratio>(dur);
}

template <
  class CharT,
  class Rep,
  class Ratio,
  class OutputIt = std::ostreambuf_iterator<CharT>
>
class fixed_put : public std::locale::facet
{
public:
  typedef CharT char_type;
  typedef std::ostreambuf_iterator<CharT> iter_type;
  
  static std::locale::id id;

  explicit fixed_put(size_t refs = 0) 
    : std::locale::facet(refs)
  {}

  iter_type put
   ( iter_type out, 
      std::ios_base& str, 
      char_type fill, 
      fixed_t<Rep, Ratio> v ) const
  {
    return do_put(out, str, fill, v);
  }
  
protected:
  ~fixed_put() {}

  virtual iter_type do_put( 
    iter_type out, 
    std::ios_base& str, 
    char_type fill, 
    fixed_t<Rep, Ratio> v 
  ) const
  {
    using namespace ::types;
    typedef std::basic_string<CharT> string_type;
    typedef fixed_t<Rep, Ratio> fixed_t;

    const auto& numpunct = 
      std::use_facet<std::numpunct<CharT>>(str.getloc());
    const auto& numput = 
     std::use_facet<std::num_put<CharT, iter_type>>
       (str.getloc());

    if (! (bool) v.rep ) {
      const string_type ovf = numpunct.falsename();
      std::copy(ovf.begin(), ovf.end(), out);
      return out;
    }

    int frac = -1;

    // count the number of frac digits needed to represent
    // without any loss

    if (frac < 0) {

      fixed_t p2(v.rep);
      frac = 0;

      // checking overflow in p2 preventing from a possible
      // infinite loop
      while((p2 *= 10) % fixed_t::one() != fixed_t::zero() 
            && (bool) p2
            )
        ++frac;

      if (! (bool) p2) {
        // the error mark (four dots)
        *out++ = numpunct.decimal_point();
        *out++ = numpunct.decimal_point();
        *out++ = numpunct.decimal_point(); 
        *out++ = numpunct.decimal_point(); 
        return out;
      }
    }
  
    const auto old_flags = str.flags();
    str.setf(std::ios_base::fixed);
    str.precision(frac);

    long double ld;
    bool lost_precision;
    v.to_long_double(ld, &lost_precision);

    // here it is!
    numput.put(out, str, fill, ld);
    str.setf(old_flags);

    if (lost_precision) {
      // the error mark (two dots)
      *out++ = numpunct.decimal_point();
      *out++ = numpunct.decimal_point();
    }
    return out;
  }
};

template <
  class CharT,
  class Rep,
  class Ratio,
  class OutputIt
>
std::locale::id fixed_put<CharT, Rep, Ratio, OutputIt>
::id;

template<
  class CharT, 
  class Rep,
  class Ratio,
  class Traits = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>& 
//
operator<<( 
  std::basic_ostream<CharT, Traits>& out,
  fixed_t<Rep, Ratio> fx
)
{
  using iterator = std::ostreambuf_iterator<CharT>;
  using fixed_put = fixed_put<CharT, Rep, Ratio, iterator>;
  std::locale locale(out.getloc(), new fixed_put);
  const fixed_put& fp = std::use_facet<fixed_put>(locale);

  const iterator end = fp.put(
    iterator(out.rdbuf()), 
    out, 
    out.fill(), 
    fx
  );

  if (end.failed())
    out.setstate(std::ios_base::badbit);

  return out;
}

#if 0
template <
  class CharT, 
  class Traits = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>& 
//
operator << 
  (std::basic_ostream<CharT, Traits>& out, 
   const put_price_type<gui::fmt::price>& p)
{
  using namespace ::types;
  typedef std::basic_string<CharT, Traits> string_type;

  if (! (bool) p.price ) {
    const string_type ovf = 
      std::use_facet<std::numpunct<CharT>>(out.getloc())
      . curr_symbol();
    out << ovf << ovf << ovf;
    return out;
  }

  int frac = p.fmt.frac_digits();

  // count the number of frac digits needed to represent
  // without any loss

  if (frac < 0) {

    fixed_t p2(p.price);
    frac = 0;

    // checking overflow in p2 preventing from a possible
    // infinite loop
    while((p2 *= 10) % 1.0_usd != 0.0_usd && (bool) p2)
      ++frac;

    if (! (bool) p2) {
      // the error mark (four dots)
      out << std::showpoint << std::showpoint 
          << std::showpoint << std::showpoint; 
      return out;
    }
  }
  
  // NB the facet of the last operation is lived
  // on the stream
  out.imbue
    (std::locale(out.getloc(), 
     p.fmt.get_facet<wchar_t>(frac))); //NB wchar_t is here

  long double ld;
  bool lost_precision;
  p.price.to_long_double(ld, &lost_precision);

  // here is it!
  out << std::put_money<long double>
    (ld / fixed_t::cents_to_bucks);

  if (lost_precision) {
    // the error mark (two dots)
    out << std::showpoint << std::showpoint; 
  }
  return out;
}

template <
  class CharT, 
  class Traits = std::char_traits<CharT>
>
std::basic_istream<CharT>& 
operator >> 
  (std::basic_istream<CharT, Traits>& in, fixed_t& p)
{
  long double ld;
  bool precision_loss_is_in_price_value;

  in >> std::get_money<long double>(ld);
  p = fixed_t::from_long_double
    (ld, &precision_loss_is_in_price_value);
  return in;
}
#endif

template<class Rep>
using percent_t = fixed_t<Rep, std::ratio<1, 100>>;

} // types

#endif
