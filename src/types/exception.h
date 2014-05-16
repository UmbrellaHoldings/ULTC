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
  using string = auto_string<max_len>;

  exception_string() {}

  explicit exception_string(const char(&message)[max_len]) 
    : msg(message) 
  {}

  exception_string(
    typename string::const_iterator begin,
    typename string::const_iterator end
  )
  {
    std::copy(begin, end, msg.begin());
  }

  const char* what() const noexcept override
  {
    return msg.data();
  }

protected:
  string msg;
};

namespace exception_ {

//! The ostream for exception message formatting
template<
  class CharT, 
  class Traits = std::char_traits<CharT>
>
class basic_ostream : public std::ios_base
{
public:
  using parent = std::ios_base; 
  basic_ostream()
  {
    using namespace std;
    // always use C locale for excpetion messages
    imbue(
      locale(
        locale::classic(), 
        new num_put<
          char, 
          std::back_insert_iterator<auto_string>
        >
      )
    );
  }
};

namespace {
// TODO single instance
basic_ostream<char> the_ostream;

}

} // exception_

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
    std::back_insert_iterator<typename parent::string>,
    Pars...
  > message;

  explicit exception_compound_message(Pars... pars)
    : parent(), message(pars...)
  {
    auto it = std::back_inserter(this->msg);
    message.stringify(
      it,
      exception_::the_ostream
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


