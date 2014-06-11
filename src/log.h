// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Bitcoin logging.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef BITCOIN_LOG_H
#define BITCOIN_LOG_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/thread.hpp>
#include <boost/filesystem/fstream.hpp>
#include "types/string.h"
#include "types/time.h"
#include "util.h"

namespace lg {

template<class CharT>
class log_traits;

template<>
class log_traits<char> : public std::char_traits<char>
{
public:
  constexpr static types::constexpr_string 
  truncated_mark()
  {
    return types::constexpr_string("*truncated*\n");
  }
  
  constexpr static char tab()
  {
    return '\t';
  }

  constexpr static char space()
  {
    return ' ';
  }
};

template <
  class CharT,
  class Traits = std::char_traits<CharT>
>
class printf_basic_stream 
  : virtual public std::basic_ios<CharT, Traits>
{
public:
  printf_basic_stream()
    : fLogTimestamps(GetBoolArg("-logtimestamps", true))
  {}

  //! @deprecated
  void vprintf(const CharT* fmt, va_list ap) noexcept;

  bool log_timestamps() const
  {
    return fLogTimestamps;
  }

protected:
  bool fStartedNewLine = true;
  bool fLogTimestamps;

  virtual boost::mutex::scoped_lock lock() 
  {
    return boost::mutex::scoped_lock(); // it means no lock
  }

  virtual void sentried(const std::function<void()>&) = 0;
};

//! The stream to log. It is a singleton.
class stream 
  : public printf_basic_stream<char>,
    public boost::filesystem::basic_ofstream<char> //, log_traits<char>>
{
public:
  static stream& instance()
  {
    static boost::once_flag of = BOOST_ONCE_INIT;
    static stream* instance = nullptr;
    boost::call_once([](){ instance = new stream(); }, of);
    assert(instance);
    return *instance;
  }

protected:
  boost::mutex::scoped_lock lock() override
  {
    return boost::mutex::scoped_lock(mx);
  }

  void sentried(const std::function<void()>& f) override
  {
    sentry s(*this);
    if (!s)
      return;

    f();
  }

private:
  stream() 
   : boost::filesystem::ofstream(
       GetDataDir() / "debug.log",
       std::ios_base::app
     )
  {
    exceptions(std::ios_base::goodbit);
    std::unitbuf(*this);
  }

  boost::mutex mx;
};

template <class CharT, class Traits>
void printf_basic_stream<CharT, Traits>
//
::vprintf(const CharT* fmt, va_list ap) noexcept
{
  using Log = log_traits<CharT>;
  sentried([this, fmt, ap]()
  {
    boost::mutex::scoped_lock scoped_lock = lock();

    // Debug print useful for profiling
    if (fLogTimestamps && fStartedNewLine)
    {
      ::times::put_time(
        *this, 
        ::times::timestamp<std::chrono::system_clock>
          ("%Y-%m-%d %H:%M:%S")
      );
      this->rdbuf()->sputc(Log::space());
    }

    if (fmt[strlen(fmt) - 1] == '\n')
      fStartedNewLine = true;
    else
      fStartedNewLine = false;

    std::array<CharT, 512> buf;

    const size_t len = vsnprintf(
      buf.data(), 
      buf.size() - Log::truncated_mark().size(), 
      fmt, 
      ap
    );

    // warn that the output is truncated
    if (len >= 
        buf.size() - Log::truncated_mark().size()
        )
    {
      Traits::copy(
        buf.end() - Log::truncated_mark().size(),
        Log::truncated_mark().begin(),
        Log::truncated_mark().size()
      );
    }

    this->rdbuf()->sputn(
      buf.data(), 
      std::min(len, buf.size())
      );
  });
}

} // lg

inline lg::stream& LOG()
{
  lg::stream& out = lg::stream::instance();
  auto a= ::times::timestamp<std::chrono::system_clock>
    ("%Y-%m-%d %H:%M:%S"); out << a << ' ';
  return out;
}

#endif
