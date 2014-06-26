/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  It will mine a new genesis block at the currency creation time.
  It was shared for everyone's profit by elbandi at https://bitcointalk.org/index.php?topic=391983.0
*/

#include <vector>
#include <memory>
#include <chrono>
#include <boost/thread.hpp>
#include "uint256.h"
#include "bignum.h"
#include "main.h"
#include "n_factor.h"
#include "hash/hash.h"
#include "log.h"
#include "algos/digishield.h"

namespace genesis {

void block::mine()
{
  using namespace std::chrono;

  // Set n cores will be used for other block mining here
  constexpr int n_cores = 6;

  CBlock& block = *this;

  DigiByte::difficulty& D = 
    DigiByte::difficulty::instance();
  
  const auto block_period = 
    D.block_period_by_design * n_cores;

  CBigNum target_difficulty = D.min_difficulty_by_design;

  // FIXME one round is not enough because the process
  // is stochastic
  unsigned start_nonce = 0;
  // <-- add a loop here

  bool found = false;

  do {

    block.nNonce = start_nonce;

    const steady_clock::time_point start = 
      steady_clock::now();

    LOG() << "Searching for genesis block..." << std::endl;
    uint256 hashTarget = target_difficulty.getuint256();
    uint256 thash;
    auto H = hash::hasher::instance(block.GetTimePoint());
     
    LOG() << "(genesis mining) start with difficulty "
          << target_difficulty.GetCompact() << std::endl;

#define LOG_BEST_HASH
#ifdef LOG_BEST_HASH
    auto best_hash = ~uint256();
#endif

    loop
    {
      thash = H->hash(block);

      if (thash <= hashTarget) {
        found = true;
        break;
      }

#ifdef LOG_BEST_HASH
      if (thash < best_hash) {
        best_hash = thash;
        LOG() << "(genesis mining) Best hash: " 
              << thash << std::endl;
        const auto passed = steady_clock::now() - start;
        if (passed > block_period + block_period / 2) {
          LOG() << "(genesis mining) "
            "target searching takes too long: "
                << duration_cast<seconds>(passed) 
                << " secs" << std::endl;
          break;
        }
      }
#else
      if ((block.nNonce & 0xFFFFF) == 0)
      {
        printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
      }
#endif
      ++block.nNonce;
      if (block.nNonce == 0)
      {
        printf("NONCE WRAPPED, incrementing time\n");
        ++block.nTime;
      }
    }
    if (found) {
      const auto passed = steady_clock::now() - start;
      if (passed < block_period / 2) {
        LOG() << "(genesis mining) "
          "target searching takes too short: "
              << duration_cast<seconds>(passed) 
              << " secs" << std::endl;
        target_difficulty >>= 1; 
        found = false;
      }
    } 
    else {
      target_difficulty <<= 1;
    }
  } while (!found);
  printf("block.nTime = %u \n", block.nTime);
  printf("block.nNonce = %u \n", block.nNonce);
  printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
}

} // genesis
