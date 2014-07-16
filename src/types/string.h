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
#include <limits>
#include <assert.h>

namespace types {

// TODO move to some right place
template<class T>
constexpr const T& min(const T& a, const T& b)
{
  return (b < a) ? b : a;
}

template<class T>
constexpr const T& max(const T& a, const T& b)
{
  return (b > a) ? b : a;
}

namespace iterators_ {

struct begin_t {};
struct end_t {};

template<
  class CharT, 
  class Pointer, 
  class Reference
>
class safe_string
{
public:
  using iterator_category = 
    std::random_access_iterator_tag;
  using value_type = CharT;
  using difference_type = int16_t;
  using size_type = uint16_t;
  using pointer = Pointer;
  using reference = Reference;
  using const_pointer = const CharT*;
  using const_reference = const CharT&;

  template<class, int16_t, class>
  friend class basic_auto_string;

  template<class C, class P, class R>
  friend class safe_string;

  bool operator==(safe_string o) const noexcept
  {
    assert(base == o.base);
    return virtual_ptr() == o.virtual_ptr();
  }

  bool operator!=(safe_string o) const noexcept
  {
    return !this->operator==(o);
  }

  bool operator<(safe_string o) const noexcept
  {
    return *this - o < 0;
  }

  bool operator>=(safe_string o) const noexcept
  {
    return *this - o >= 0;
  }

  bool operator>(safe_string o) const noexcept
  {
    return *this - o > 0;
  }

  bool operator<=(safe_string o) const noexcept
  {
    return *this - o <= 0;
  }

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
      ovf += n;
    }
    return *this;
  }

  safe_string operator++(int) noexcept
  {
    safe_string copy(*this);
    ++(*this);
    return copy;
  }

  difference_type operator-(safe_string o) const noexcept
  {
    assert(base == o.base);
    return virtual_ptr() - o.virtual_ptr();
  }

  // cast to const_iterator
  operator 
  safe_string<CharT, const_pointer, const_reference>() const 
    noexcept
  {
    using const_iterator = 
      safe_string<CharT, const_pointer, const_reference>;
    return const_iterator(base, n, idx, ovf);
  }

  static_assert(
    sizeof(size_type) < sizeof(size_t),
    "unable to correctly implement virtual_ptr()"
  );
  const_pointer virtual_ptr() const noexcept
  {
    return base + idx + ovf;
  }

  //protected:  //TODO problem with friend basic_auto_string in clang
  safe_string(
    pointer base_, 
    int16_t n_, 
    size_type idx_, 
    int16_t ovf_
  )  noexcept 
    : base(base_), idx(idx_), ovf(ovf_), n(n_) 
  {
    assert(n >= 0);
  }

  safe_string(pointer base_, int16_t n_, begin_t) noexcept 
    : safe_string(base_, n_, 0, 0)
  {}

  safe_string(pointer base_, int16_t n_, end_t) noexcept 
    : safe_string(base_, n_, 0, n_)
  {}

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

  constexpr const value_type* data() const 
  { 
    return arr; 
  }

  constexpr const value_type* c_str() const 
  { 
    return arr; 
  }

  const value_type* begin() const
  {
    return arr;
  }

  const value_type* end() const
  {
    return arr + len;
  }

private:
  const size_type len;
  const value_type* const arr;
};

typedef constexpr_basic_string<char> constexpr_string;
typedef constexpr_basic_string<wchar_t> constexpr_wstring;

/**
 * It is usefull for parsing template literal operators.
 */
template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  CharT...
> 
class basic_meta_string;

template <
  class CharT,
  class Traits
> 
class basic_meta_string<CharT, Traits>
{
public:
  typedef uint16_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  constexpr static size_type size() { return 0; }

  operator std::string() const
  {
    return std::string();
  }

  template<class OutputIt>
  static void copy_to(OutputIt out)
  {
  }
};

template <
  class CharT,
  class Traits,
  CharT C0,
  CharT... CS
> 
class basic_meta_string<CharT, Traits, C0, CS...>
  : public basic_meta_string<CharT, Traits, CS...>
{
  using parent = basic_meta_string<CharT, Traits, CS...>;
public:
  typedef uint16_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  constexpr static size_type size()
  { 
    return parent::size() + 1;
  }

  operator std::string() const
  {
    std::string res(size(), '\0');
    copy_to(res.begin());
    return res;
  }

  //! Copy the string to the output iterator
  template<class OutputIt>
  static void copy_to(OutputIt out)
  {
    *out++ = C0;
    parent::copy_to(out);
  }
};

template<char... cs>
using meta_string = basic_meta_string
  <char, std::char_traits<char>, cs...>;

template<wchar_t... wcs>
using meta_wstring = basic_meta_string
  <wchar_t, std::char_traits<wchar_t>, wcs...>;


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

//! A string with an automatic (cycled) storage.
//! It maintains two sizes: size of the buffer and size of
//! the string (the used part of the buffer). 
//! So, size() == end() - begin()
//! buf_size() - 1 = buf_end() - begin()
//! end() can be greater than buf_end() (it's cycled).

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
   basic_auto_string_traits<CharT, Traits>
     ::const_iterator;

  basic_auto_string() noexcept : cur_end(begin()) 
  {
    m[N-1] = 0;
  }

  basic_auto_string(const CharT(&str)[N]) noexcept
    : cur_end(end())
  {
    traits_type::copy(m.data(), str, N);
  }

  void swap(basic_auto_string& o) noexcept
  {
    m.swap(o.m);
  }

  //! Returns the size of buffer with ending 0, so
  constexpr size_type buf_size() const 
  { 
    return N; 
  }

  size_type size() const 
  {
    return end() - begin();
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
    return cur_end;
  }

  const_iterator end() const noexcept
  {
    return cur_end;
  }

  //! Returns the end of buffer, not only filled size
  iterator buf_end() noexcept 
  {
    return iterator(m.data(), N-1, iterators_::end_t());
  }

  const_iterator buf_end() const noexcept
  {
    return const_iterator(
      m.data(), N-1, iterators_::end_t()
    );
  }

  const value_type* data() const
  {
    assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return m.data();

    // assert(*end() == 0);
    // *end() = 0;
    // Don't do it, you can break a message which overruns
    // the buffer.
  }

  const value_type* c_str() const
  {
    assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return data();
  }

  void push_back(value_type ch) noexcept
  {
    *cur_end++ = ch;
  }

protected:
  // it is mutable - padding with '\0' is allowed
  mutable std::array<CharT, N> m;

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

  // TODO open modes
  basic_auto_stringbuf() 
  {
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size());
    this->setp(p, p + s.size());
  }

  // TODO open modes
  basic_auto_stringbuf(const CharT(&str)[N]) 
    : s(str) 
  {
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size());
    this->setp(p, p + s.size());
  }
  
  // TODO open modes
  basic_auto_stringbuf(const string& s_) 
  {
    str(s_);
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size());
    this->setp(p, p + s.size());
  }

  string& str() noexcept 
  {
    return s; 
  }

  const string& str() const noexcept 
  { 
    return s; 
  }

  void str(const string& s_) noexcept 
  {
    s = s_;
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size()); // open modes ?
    this->setp(p, p + s.size());
  }

protected:
  // put area

  int_type overflow(int_type ch = Traits::eof()) override
  {
    if (Traits::eq_int_type(ch, Traits::eof()))
      return ch;

    if (s.end() < s.buf_end()) {
      s.push_back(ch);
      return ch;
    }
    else
      return Traits::eof();
  }

  // get area

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

  // positioning

  pos_type seekoff
    ( 
      off_type off, 
      std::ios_base::seekdir dir,
      std::ios_base::openmode which = std::ios_base::in
     ) override
  {
    using namespace std;
    const pos_type end_pos = this->egptr()- this->eback();
    off_type abs_pos(0);

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

    if (!(bool) abs_pos || abs_pos < 0) 
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
    const pos_type end_pos = this->egptr()- this->eback();

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
class stringifier_t;

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
    try {
      std::copy(ptr, ptr + N - 1, out);
    }
    catch(...) {
      *out++ = '*';
    }
  }

protected:
  const char_type *const ptr;
};

// for a basic_meta_string
template<class CharT, class Traits, CharT... CS>
struct len_t<basic_meta_string<CharT, Traits, CS...>&&>
{
  static constexpr size_t max_length = 
    basic_meta_string<CharT, Traits, CS...>::size();
};

template<class OutIt, class CharT, class Traits, CharT... CS>
class stringifier_t<
  OutIt,
  basic_meta_string<CharT, Traits, CS...>&&
>
{
  using string = basic_meta_string<CharT, Traits, CS...>;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string) noexcept {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    try {
      const std::string s = string();
      std::copy(s.begin(), s.end(), out);
    }
    catch(...) {
      *out++ = '*';
    }
  }
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
    try {
      using namespace std;
      use_facet<num_put<char_type, OutIt>>(st.getloc())
        . put(out, st, ' ', val);
    }
    catch (...) {
      *out++ = '*';
    }
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

  stringifier_t(Arg0&& arg0, Args&&... args) noexcept
    : head(std::forward<Arg0>(arg0)), 
      tail(std::forward<Args>(args)...)
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

template<class OutIt, class... Args>
using compound_message_t = 
  compound_message_::stringifier_t<OutIt, Args...>;

} // types

#endif
