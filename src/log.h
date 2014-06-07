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
#include "types/string.h"
#include "types/time.h"
#include "util.h"

namespace lg {

#if 0
template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Allocator = std::allocator<CharT>
>
class basic_filebuf
  : public std::basic_filebuf<CharT, Traits, Allocator>
{
  typedef std::basic_filebuf<CharT, Traits, Allocator> 
    parent;

  friend class stream;

public:
  typedef CharT char_type;
  typedef Traits traits_type;
  typedef typename Traits::int_type int_type;
  typedef typename Traits::pos_type pos_type;
  typedef typename Traits::off_type off_type;

protected:
  std::streamsize xsputn( 
    const char_type* s, 
    std::streamsize count
  ) override
  {
    boost::mutex::scoped_lock scoped_lock(mx);
    parent::xsputn(s, coint);
  }

  //TODO sputc is not protected by mutex. In the true logging system
  //need to redesign it for using separate buffer for each thread
  //without mutex.

  boost::recursive_mutex mx;
};
#endif

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
};

#if 0
template <
  class CharT,
  class Traits = log_traits<CharT>
>
class printf_basic_stream 
  : public std::basic_ostream<CharT, Traits>
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
    return boost::mutex::scoped_lock(); // no lock?
  }
};
#endif

//! The stream to log. It is a singleton.
class stream 
  : //public printf_basic_stream<char>,
    public std::basic_ofstream<char> //, log_traits<char>>
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

  void vprintf(const char* fmt, va_list ap) noexcept;

  bool log_timestamps() const
  {
    return fLogTimestamps;
  }

protected:
  bool fStartedNewLine = true;
  bool fLogTimestamps;

  boost::mutex::scoped_lock lock() //override
  {
    return boost::mutex::scoped_lock(mx);
  }

private:
  stream() 
   :
#if 1
    std::ofstream(
        (GetDataDir() / "debug.log").c_str(), std::ios_base::app
      ),
#endif
  fLogTimestamps(GetBoolArg("-logtimestamps", true))
  {
//    std::cout << rdstate() << std::endl;
//    open((GetDataDir() / "debug.log").c_str());
    std::cout << rdstate() << std::endl;;
//    rdbuf()->pubsetbuf(0, 0);
    std::cout << rdstate() << std::endl;;
    exceptions(std::ios_base::goodbit);
    std::cout << rdstate() << std::endl;;
  }

  boost::mutex mx;
};

#if 0
template <class CharT, class Traits>
void printf_basic_stream<CharT, Traits>
//
::vprintf(const CharT* fmt, va_list ap) noexcept
{
#else
inline void stream//<CharT, Traits>
//
::vprintf(const char* fmt, va_list ap) noexcept
{
  using CharT = char;
  using Traits = std::char_traits<char>; //log_traits<char>;
  using Log = log_traits<char>;
#endif
  typename std::basic_ostream<CharT, Traits>::sentry 
    sentry(*this);
  if (!sentry)
    return;

  boost::mutex::scoped_lock scoped_lock = lock();

  // Debug print useful for profiling
  if (fLogTimestamps && fStartedNewLine)
    *this << times::timestamp<std::chrono::system_clock>
      ("%Y-%m-%d %H:%M:%S");
  this->rdbuf()->sputc(Log::tab());

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
#if 1
    Traits::copy(
      buf.end() - Log::truncated_mark().size(),
      Log::truncated_mark().begin(),
      Log::truncated_mark().size()
    );
#else
    std::copy(
      Log::truncated_mark().begin(), 
      Log::truncated_mark().end(), 
      buf.end() - Log::truncated_mark().size()
    );
#endif
  }

  this->rdbuf()->sputn(
    buf.data(), 
    std::min(len, buf.size())
  );
  this->rdbuf()->pubsync();
}

}

#endif
