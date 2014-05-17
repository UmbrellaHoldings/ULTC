// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_EXCEPTION_H
#define COHORS_TYPES_EXCEPTION_H

#include <iostream>
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

  exception_string(const exception_string& o)
    : exception_string()
  {
    msg.str(o.msg.str());
  }

  exception_string(
    typename string::const_iterator begin,
    typename string::const_iterator end
  )
  {
    std::copy(begin, end, msg.str().begin());
  }

  exception_string& operator=(const exception_string& o)
  {
    msg.str(o.msg.str());
    return *this;
  }

  const char* what() const noexcept override
  {
    return msg.str().data();
  }

protected:
  stringbuf msg;
};

#if 0
namespace exception_ {

//! The ostream for exception message formatting
template<
  class CharT, 
  class Traits = std::char_traits<CharT>
>
class basic_ostream : public std::ios_base
{
public:
  basic_ostream()
  {
    // always use C locale for excpetion messages
    imbue(std::locale::classic());
  }
};

namespace {
// TODO single instance
basic_ostream<char> the_ostream;

}

} // exception_
#endif

template<class... Pars>
class exception_compound_message 
  : public exception_string<
      compound_message_max_length<Pars...>() + 1
    >
{
  using parent = exception_string<
    compound_message_max_length<Pars...>() + 1
  >;

public:
  const compound_message_t<
    std::ostreambuf_iterator<char>,
    Pars...
  > message;

  explicit exception_compound_message(Pars... pars)
    : parent(), message(pars...)
  {
    auto it = std::ostreambuf_iterator<char>(&this->msg);
    message.stringify(
      it,
      std::cout //exception_::the_ostream
    );
    *it = '\0';
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


