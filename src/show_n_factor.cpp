/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2014 Sergei Lodyagin 
 
  The utility for print Vertcoin GetNfactor.
*/

#include <iostream>
#include <cmath>
#include <tuple>
#include "types/time.h"
#include "btc_time.h"

using namespace std;
using namespace std::chrono;
using namespace curr;

std::tuple<size_t, unsigned, unsigned> 
GetNfactor(coin::time::block::time_point block_time);

using namespace coin::time::block;

using days = duration<clock::rep, ratio<3600 * 24>>;
using common_years = 
  duration<clock::rep, ratio<3600 * 24 * 365>>;

//! shows n_factor for the specified time point
void show(coin::time::block::time_point now)
{
  size_t N = 0;
  unsigned r = 0, p = 0;
  tie(N, r, p) = GetNfactor(now);
  std::cout << put_time(clock::to_system_clock(now), "%c")
    << "\t<" << N << ", " << r << ", " << p << ">\t"
    << (size_t) 128 * N * r * p / (1024 * 1024) << "MiB"
    << std::endl;
}

//! shows n_factor for the specified period
void show(
  const coin::time::block::time_point from, 
  const coin::time::block::time_point to
)
{
  auto now = from;
  show(now);
  auto last_factor = GetNfactor(now);
  while (now <= to) {
    while (GetNfactor(now) == last_factor) 
      now += hours(24);
    last_factor = GetNfactor(now);

    show(now);
  }
}

int main(int argc, char* argv[])
{
  auto now = clock::now();
  std::cout << "Date\tGetNfactor result" << std::endl;
  show(now, now + common_years(21));
}
