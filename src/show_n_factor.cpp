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

int main(int argc, char* argv[])
{
  auto now = clock::now();
  std::cout << "Date\tGetNfactor result" << std::endl;

  show(now);
  auto last_factor = GetNfactor(now);
  for (int k = 0; k < 600; k++) {
    while (GetNfactor(now) == last_factor) 
      now += hours(24);
    last_factor = GetNfactor(now);

    show(now);
  }
}
