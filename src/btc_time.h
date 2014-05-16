// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Bitcoin time.
 *
 * @author Wladimir J. van der Laan
 * @author Sergei Lodyagin
 */

#ifndef BITCOIN_TIME_H
#define BITCOIN_TIME_H

#include <ostream>
#include <chrono>
#include <ratio>
#include "types/fixed.h"

namespace coin {
namespace time {

namespace block {

struct clock
{
  using rep        = std::chrono::system_clock::rep;
  using period     = std::ratio<1>;
  using duration   = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<clock>;
  
  static constexpr bool is_steady = false;

  static time_point now()
  {
    using namespace std::chrono;

    //FIXME, it is test only
    return time_point(
      duration_cast<seconds>(
        system_clock::now().time_since_epoch()
      )
    );
  }

  static constexpr std::chrono::system_clock::time_point
  to_system_clock(time_point local)
  {
    using namespace std::chrono;

    return system_clock::time_point(
      duration_cast<system_clock::duration>
        (local.time_since_epoch()) // both times use the
                                   // same epoch
    );
  }
};

using seconds = 
  std::chrono::duration<clock::rep, std::ratio<1>>;
using minutes = 
  std::chrono::duration<clock::rep, std::ratio<60>>;
using hours = 
  std::chrono::duration<clock::rep, std::ratio<60 * 60>>;
using days = 
  std::chrono::duration<clock::rep, std::ratio<60*60*24>>;

using time_point = clock::time_point;


//using duration = clock::duration;
#if 0
std::ostream& 
//
operator<<( 
  std::ostream& out,
  duration dur
)
{
  return out << types::fixed_t<duration::rep, duration::period>(dur);
}
#else
#endif

} // block
} // time
} // coin

namespace std { namespace chrono {

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
  duration<Rep, Ratio> dur
)
{
  return out << types::fixed_t<Rep, Ratio>(dur);
}

} // chrono
} // std

#endif
