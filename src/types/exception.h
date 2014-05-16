// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_EXCEPTION_H
#define COHORS_TYPES_EXCEPTION_H

#include <tuple>
#include <algorithm>
#include <iterator>
#include "types/string.h"

namespace types {

template<uint16_t max_len>
class exception_string : public virtual std::exception
{
public:
  using stringbuf = auto_stringbuf<max_len>;
  using string = typename stringbuf::string;

  exception_string() {}

  explicit exception_string(const char(&message)[max_len]) 
    : msg(message) 
  {}

  exception_string(
    typename string::const_iterator begin,
    typename string::const_iterator end
  )
  {
    std::copy(begin, end, msg.s.begin());
  }

  const char* what() const noexcept override
  {
    return msg.data();
  }

protected:
  stringbuf msg;
};

namespace exception_ {

//! The ostream for exception message formatting
template<
  class CharT, 
  class Traits = std::char_traits<CharT>
>
class basic_ostream : public std::ios_base//<CharT, Traits>
{
public:
  using parent = std::ios_base; //<CharT, Traits>;
  basic_ostream()
  {
    // always use C locale for excpetion messages
    imbue(std::locale::classic());
  }
};

namespace {

basic_ostream<char> the_ostream;

}

} // exception_

template<class... Pars>
class exception_compound_message 
  : public exception_string<
      compound_message_max_length<Pars...>() + 1
    >
{
public:
  using exception_base = exception_string<
    compound_message_max_length<Pars...>() + 1
  >;

  const compound_message_t<
    std::ostreambuf_iterator<char>,
    Pars...
  > message;

  explicit exception_compound_message(Pars... pars)
    : exception_base(), message(pars...)
  {
    message.stringify(
      std::ostreambuf_iterator<char>(&exception_base.msg),
      exception_::the_ostream
    );
  }
};

namespace formatted_ {

template<class Exception, class... Args>
struct exception
  : Exception, 
    exception_compound_message<Args...> 
{
  exception(Args... args) 
    : exception_compound_message<Args...>(args...) {}
};

}

template<class Exception, class... Args>
auto exception(Args&&... args)
  -> formatted_::exception<Exception, Args&&...>
{
  return formatted_::exception<Exception, Args&&...>
    (std::forward<Args>(args)...);
}

} // types

#endif


