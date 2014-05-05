// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Calculate the "N-factor", it is yacoin/vertcoin concept
 * originally, see 
 * http://www.followthecoin.com/interview-creator-vertcoin/
 *
 * @author Sergei Lodyagin
 */

#include <tuple>
#include <chrono>
#include <cmath>
#include "types/time.h"
#include "btc_time.h"

//! Returns tuple with scrypt N,r,p parameters.
//! It differs from vertcoin. The assumption is doubling
//! a number of transistors (CPU cores) every 18 monthes
//! (Moore's law, N-factor) but increasing the speed of
//! CPU only by 10-20% (we use 15%).
//! Why increase N (amount of memory) but not p (a scrypt
//! parrallelism parameter)?  Because increasing of p not
//! protect us against ASIC/GPUs and increasing of CPU
//! cores means decreasing of block time checking (when a
//! node goes online and checks generated block in the
//! whole period from it was online last time). The block
//! checking period is a real limitation of scrypt memory
//! usage parameter (it is 128*N*r*p, the greater
//! parameter means better GPU protection but at the same
//! time greater block checking time on a single core
//! which is near proportional to N*r).
std::tuple<size_t, unsigned, unsigned> 
GetNfactor(coin::time::block::time_point block_time) 
{
  using namespace coin::time::block;
  using namespace std::chrono;
  using namespace std;

  using days = duration<clock::rep, ratio<3600 * 24>>;
  using avg_months = duration
    <clock::rep, ratio<3600 * 24 * 30>>;

  constexpr size_t MiB = 1024 * 1024;

  //! The function birth time
  static const auto birth_time =
    coin::time::block::time_point(
      days(curr::time::howard_hinnant::days_from_civil
        (2014, 05, 05))
    );

  if (block_time < birth_time)
    // invalid block time, use initial values
    block_time = birth_time;

  // he said "every 18 months"
  constexpr auto moore_period = avg_months(18);

  const int moore_steps = 
    duration_cast<avg_months>(block_time - birth_time)
    / moore_period; 

  const auto last_step_time =
    birth_time + moore_steps * moore_period;

  // the scrypt parameters
  constexpr int p = 1; // increasing is not protect us
  constexpr size_t initial_mem = 1 * MiB;
  constexpr unsigned initial_r = 8;
  size_t mem = (initial_mem << moore_steps);
  if (mem < initial_mem) mem = initial_mem;

  // today min amount of scrypt memory per device
  constexpr size_t birth_total_memory = 128 * MiB; 
  // today min amount of CPU cores
  constexpr int birth_cores = 2;

  // assume memory access speed is not increasing, so
  // limit block load time to 2.5 mins 
  constexpr size_t max_mem = 
    (size_t) 256 * 1024 * 1024 * 1024;
  if (mem > max_mem) mem = max_mem;

  size_t moore_cores = (birth_cores << moore_steps);
  if (moore_cores < birth_cores) moore_cores = birth_cores;
  // memory used by scrypt when use moore_cores
  size_t moore_total_memory = 
    (birth_total_memory << moore_steps);
  if (moore_total_memory < birth_total_memory)
    moore_total_memory = birth_total_memory;

  if (mem > moore_total_memory)
    mem = moore_total_memory;

  if (mem / initial_mem > moore_cores)
    mem = initial_mem * moore_cores;

  // CPU speed grow 15% per moore_step, 
  // it is twiced in 5 steps
  const unsigned r0 = (initial_r << moore_steps / 5);
  const auto last_r_step_time = last_step_time - 
    moore_period * (moore_steps % 5);
  const unsigned r = r0 
    + r0 * (block_time - last_r_step_time) 
    / (5 * moore_period);

  // N must be power of 2
  const size_t N = mem / (128 * r0 * p);
//  size_t N = 1 << (sizeof(N1) * 8 - __builtin_clz(N1) - 1);
  //if (N == 0) N = 1024;
  return make_tuple(N, r, p);
}

