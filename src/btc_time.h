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

#include <chrono>
#include <ratio>

namespace coin {
namespace time {
namespace block {

struct clock
{
  using rep = std::chrono::system_clock::rep;
  using period = std::ratio<1>;
  using duration = std::chrono::duration<rep, period>;
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

using time_point = clock::time_point;

} // block
} // time
} // coin

#endif
