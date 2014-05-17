// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Different string types.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_STRING_H
#define COHORS_TYPES_STRING_H

#include <ios>
#include <streambuf>
#include <string>
#include <array>
#include <cstdint>
#include <iterator>
#include <assert.h>

namespace types {

namespace iterators_ {

struct begin_t {};
struct end_t {};

template<
  class CharT, 
//  size_t N, 
  class Pointer, 
  class Reference
>
class safe_string
{
  template<class, int16_t, class>
  friend class basic_auto_string;

public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = CharT;
  using difference_type = int16_t;
  /*static_assert(
    N <= std::numeric_limits<difference_type>::max(),
    "types::iterators_::safe_string N is to high"
  );*/
  using size_type = uint16_t;
  using pointer = Pointer;
  using reference = Reference;
  using const_pointer = const CharT*;
  using const_reference = const CharT&;

  reference operator*() noexcept
  {
    return base[idx];
  }

  const_reference operator*() const noexcept
  {
    return base[idx];
  }

  safe_string& operator++() noexcept
  {
    if (__builtin_expect(idx++ >= n, 0))
    {
      idx = 0;
      ++ovf;
    }
    return *this;
  }

  safe_string operator++(int) noexcept
  {
    safe_string copy(*this);
    ++(*this);
    return copy;
  }

protected:
  safe_string(pointer base_, int16_t n_, begin_t) noexcept 
    : base(base_), idx(0), ovf(0), n(n_) 
  {
    assert(n >= 0);
  }

  safe_string(pointer base_, int16_t n_, end_t) noexcept 
    : base(base_), idx(0), ovf(1), n(n_)
  {
    assert(n >= 0);
  }

  pointer base;
  size_type idx;
  int16_t ovf;
  int16_t n;
};

} // iterators_

/**
 * Just constexpr_basic_string. It is used for "wrap"
 * string literals and not pass strings with unpredicted
 * length to other functions (e.g., streams).
 */
template <
  class CharT,
  class Traits = std::char_traits<CharT>
> 
class constexpr_basic_string 
{
public:
  typedef uint32_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  template<std::uint32_t N>
  constexpr constexpr_basic_string(const char(&str)[N])
    : len(N-1), arr(str)
  {
  }

  constexpr size_type size() const { return len; }
  constexpr const value_type* data() const { return arr; }
  constexpr const value_type* c_str() const { return arr; }

private:
  const size_type len;
  const value_type* const arr;
};

typedef constexpr_basic_string<char> constexpr_string;
typedef constexpr_basic_string<wchar_t> constexpr_wstring;

template<
  class CharT,
  class Traits = std::char_traits<CharT>
>
struct basic_auto_string_traits
{
  using iterator = iterators_::safe_string
    <CharT, CharT*, CharT&>;
  using const_iterator = iterators_::safe_string
    <CharT, const CharT*, const CharT&>;
};

using auto_string_traits = basic_auto_string_traits<char>;
using auto_wstring_traits = 
  basic_auto_string_traits<wchar_t>;

//! A string in an automatic storage
template <
  class CharT,
  int16_t N,
  class Traits = std::char_traits<CharT>
> 
class basic_auto_string 
{
  static_assert(
    N > 0, 
    "types::basic_auto_string: invalid size"
  );

public:
  using traits_type = Traits;
  using value_type = CharT;
  using size_type = uint16_t;
  using difference_type = int16_t;
  using iterator = typename 
    basic_auto_string_traits<CharT, Traits>::iterator;
  using const_iterator = typename 
   basic_auto_string_traits<CharT, Traits>::const_iterator;

  basic_auto_string() noexcept : cur_end(begin()) {}

  basic_auto_string(const CharT(&str)[N]) noexcept
    : cur_end(end())
  {
    traits_type::copy(m.data(), str, N);
  }

  void swap(basic_auto_string& o) noexcept
  {
    m.swap(o.m);
  }

  iterator begin() noexcept
  {
    return iterator(m.data(), N-1, iterators_::begin_t());
  }

  const_iterator begin() const noexcept
  {
    return const_iterator(
      m.data(), N-1, iterators_::begin_t()
    );
  }

  iterator end() noexcept 
  {
    return iterator(m.data(), N-1, iterators_::end_t());
  }

  const_iterator end() const noexcept
  {
    return const_iterator(
      m.data(), N-1, iterators_::end_t()
    );
  }

  const value_type* data() const
  {
    return m.data();
  }

  const value_type* c_str() const
  {
    return data();
  }

  void push_back(value_type ch) noexcept
  {
    *cur_end++ = ch;
  }

protected:
  std::array<CharT, N> m;
  iterator cur_end;
};

// TODO
template<
  class CharT, 
  class Traits
>
class basic_auto_string<CharT, 0, Traits>;

template<int16_t N>
using auto_string = basic_auto_string<char, N>;

template<int16_t N>
using auto_wstring = basic_auto_string<wchar_t, N>;

template <
  class CharT,
  int16_t N,
  class Traits = std::char_traits<CharT>
>
class basic_auto_stringbuf 
  : public std::basic_streambuf<CharT, Traits>
{
  typedef std::basic_streambuf<CharT, Traits> parent;

public:
  typedef CharT char_type;
  typedef Traits traits_type;
  typedef typename Traits::int_type int_type;
  typedef typename Traits::pos_type pos_type;
  typedef typename Traits::off_type off_type;
  typedef basic_auto_string<CharT, N, Traits> string;

  basic_auto_stringbuf() 
  {
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + N - 1);
    this->setp(p, p + N - 1);
  }

  basic_auto_stringbuf(const CharT(&str)[N]) : s(str) 
  {
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + N - 1);
    this->setp(p, p + N - 1);
  }
  
  basic_auto_stringbuf(const string& s_) 
    : basic_auto_stringbuf()
  {
    str(s_);
  }

#if 0
  basic_auto_stringbuf(const basic_auto_stringbuf& o)
    : basic_auto_stringbuf(o.str())
  {
    imbue(o.getloc());
  }

  basic_auto_stringbuf& operator=(
    const basic_auto_stringbuf& o
  )
  {
    str(o.str());
    imbue(o.getloc());
  }
#endif

  string& str() noexcept { return s; }

  const string& str() const noexcept { return s; }

  void str(const string& s_) noexcept 
  {
    s = s_;
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + N - 1);
    this->setp(p, p + N - 1);
  }

protected:
  std::streamsize showmanyc() override
  {
    return this->egptr() - this->gptr();
  }

  int_type underflow() override
  {
    return (showmanyc()) 
      ? Traits::to_int_type(*this->gptr()) 
      : Traits::eof();
  }

  pos_type seekoff
    ( 
      off_type off, 
      std::ios_base::seekdir dir,
      std::ios_base::openmode which = std::ios_base::in
     ) override
  {
    using namespace std;
    const pos_type end_pos = this->egptr() - this->eback();
    safe<off_type> abs_pos(0);

    switch((uint32_t)dir) {
      case ios_base::beg: 
        abs_pos = off;
        break;
      case ios_base::end:
        abs_pos = end_pos + off;
        break;
      case ios_base::cur:
        abs_pos = this->gptr() - this->eback() + off;
        break;
    }

    if (!(bool) abs_pos || abs_pos < safe<off_type>(0)) 
      // the rest will be checked in seekpos
      return pos_type(off_type(-1));
    
    return seekpos((off_type) abs_pos);
  }

  pos_type seekpos
    ( 
      pos_type pos, 
      std::ios_base::openmode which = std::ios_base::in
     ) override
  {
    const pos_type end_pos = this->egptr() - this->eback();

    if (pos > end_pos || which & std::ios_base::out)
      return pos_type(off_type(-1));

    this->setg
      (this->eback(), this->eback() + pos, this->egptr());
    return pos;
  }

  string s;
};

template<int16_t N>
using auto_stringbuf = basic_auto_stringbuf<char, N>;

template<int16_t N>
using auto_wstringbuf = basic_auto_stringbuf<wchar_t, N>;

namespace compound_message_ {

template<class... Args>
struct len_t;

template<class OutIt, class... Args>
struct stringifier_t;

// an empty tail case
template<>
struct len_t<>
{
  static constexpr size_t max_length = 0;
};

template<class OutIt>
class stringifier_t<OutIt>
{
public:
  void stringify(OutIt out, std::ios_base&) const noexcept
  {}
};

// for a string literal
template<class CharT, size_t N>
struct len_t<const CharT(&)[N]>
{
  static constexpr size_t max_length = N - 1;
};

template<class OutIt, size_t N>
class stringifier_t<
  OutIt,
  const typename OutIt::char_type(&)[N]
>
{
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(const char_type(&s)[N]) noexcept
    : ptr(s) 
  {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    std::copy(ptr, ptr + N - 1, out);
  }

protected:
  const char_type *const ptr;
};

// for long double
// TODO enable_if(type class)
template<>
struct len_t<long double&>
{
  static constexpr size_t max_length = 
    std::numeric_limits<long double>::digits // mantissa
    + constexpr_string("-1.e-123").size();
};

template<class OutIt>
class stringifier_t<OutIt, long double&>
{
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(long double v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base& st) 
    const noexcept
  {
    using namespace std;
    // TODO check noexcept 
    // & no memory allocation condition
    const auto& np = 
      use_facet<num_put<char_type, OutIt>>(st.getloc());
    
    np.put(out, st, ' ', val);
  }

protected:
  const long double val;
};

// recursive
template<class Arg0, class... Args>
struct len_t<Arg0, Args...> : len_t<Arg0>, len_t<Args...>
{
  using head = len_t<Arg0>;
  using tail = len_t<Args...>;

  static constexpr size_t max_length = 
    head::max_length + tail::max_length;
};

template<class OutIt, class Arg0, class... Args>
class stringifier_t<OutIt, Arg0, Args...>
  : public stringifier_t<OutIt, Arg0>,
    public stringifier_t<OutIt, Args...>
{
public:
  using head = stringifier_t<OutIt, Arg0>;
  using tail = stringifier_t<OutIt, Args...>;

  stringifier_t(Arg0 arg0, Args... args) noexcept
    : head(arg0), tail(args...)
  {}

  void stringify(OutIt out, std::ios_base& st) 
    const noexcept
  {
    head::stringify(out, st);
    tail::stringify(out, st);
  }
};

} // compound_message_

template<class... Args>
constexpr size_t compound_message_max_length()
{
  return compound_message_::len_t<Args...>::max_length;
}

#if 1
template<class OutIt, class... Args>
using compound_message_t = 
  compound_message_::stringifier_t<OutIt, Args...>;
#else
// TODO the same as stringifier_t ?
template<class OutIt, class... Args>
class compound_message_t 
{
public:
  using stringifier_t = compound_message_::stringifier_t<
    std::ostreambuf_iterator<char>,
    Args...
  >;

  static constexpr typename string::size_type max_length = 
    stringifier_t::max_length;

  explicit compound_message_t(OutIt out, Args... args) 
    noexcept
    : stringifier_t(out, args...)
  {}
};
#endif

} // types

#endif
