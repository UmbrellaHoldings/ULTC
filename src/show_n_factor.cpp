/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2014 Sergei Lodyagin 
 
  The utility for print Vertcoin GetNfactor.
*/

#include <iostream>
#include <chrono>
#include <cmath>
#include "types/time.h"

using namespace std;
using namespace std::chrono;
using namespace curr;

int GetNfactor(int64_t nTimestamp);

int main(int argc, char* argv[])
{
  const auto now = system_clock::now();
  const auto avg_year = 365 * hours(24);
  static const int n_years = 50;
  std::cout << "Date\tGetNfactor result" << std::endl;
  for (int i = 0; i <= n_years; i++)
  {
    const auto p = now + i * avg_year;
    const auto N = GetNfactor(
       duration_cast<seconds>(p.time_since_epoch())
       . count()
    );
    std::cout << put_time(p, "%c") << '\t' 
      << N << '\t'
      << pow(2, N) 
      << std::endl;
  }
}
