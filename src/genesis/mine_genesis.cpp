/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  It will mine a new genesis block at the currency creation time.
  It was shared for everyone's profit by elbandi at https://bitcointalk.org/index.php?topic=391983.0
*/

#include <forward_list>
#include <memory>
#include <chrono>
#include <boost/thread.hpp>
#include "types/abstract_iterator.h"
#include "uint256.h"
#include "bignum.h"
#include "main.h"
#include "n_factor.h"
#include "hash/hash.h"
#include "log.h"
#include "pars.h"
#include "algos/retarget.h"
#include "btc_time.h"

//#define LOG_BEST_HASH
//#define DO_NOT_CALIBRATE_DIFFICULTY

namespace {

// types used for search desired block difficulty

using difficulty_list = std::forward_list<block::info_type>;

using difficulty_iterator = 
  types::virtual_iterator::const_forward
    <difficulty_list::const_iterator>;

}

namespace genesis {

void block::mine()
{
  using namespace std::chrono;

  // Set n cores will be used for other block mining here
  const int n_cores = pars::testnet_switch(
    types::make_pair(6, 1)
  );

  CBlock& block = *this;

  retarget::difficulty& D = 
    retarget::difficulty::instance();
  
  const auto block_period = 
    D.block_period_by_design * n_cores;

  compact_bignum_t target_difficulty = 
    D.min_difficulty_by_design;

  // FIXME one round is not enough because the process
  // is stochastic
  unsigned start_nonce = 0;
  // <-- add a loop here

  bool found = false;

  difficulty_list d_list;
  d_list.emplace_front(
    ::block::info_type{
      0, 
      0, 
      coin::times::block::clock::now()
    }
  );
  const auto d_list_bottom = d_list.cbegin();

  do {

    block.nNonce = start_nonce;

    const auto start = coin::times::block::clock::now();

    LOG() << "Searching for genesis block..." << std::endl;
    uint256 hashTarget = 
      CBigNum(target_difficulty).getuint256();
    uint256 thash;

    auto H = hash::hasher::instance(block.GetTimePoint());
     
    LOG() << "(genesis mining) start with difficulty "
          << target_difficulty << std::endl;
    block.nBits = target_difficulty;

    auto best_hash = ~uint256();

    loop
    {
      thash = H->hash(block);

      if (thash <= hashTarget) {
        found = true;
        break;
      }

      if (thash < best_hash) {
        best_hash = thash;
#ifdef LOG_BEST_HASH
        LOG() << "(genesis mining) Best hash: " 
              << thash << std::endl;
#endif
#ifndef DO_NOT_CALIBRATE_DIFFICULTY
        const auto now = coin::times::block::clock::now();
        const auto passed = now - start;
        if (passed > block_period + block_period / 2) {
          LOG() << "(genesis mining) "
            "target searching takes too long: "
                << duration_cast<seconds>(passed) 
                << " secs" << std::endl;
          d_list.push_front({
            d_list.front().height + 1,
            target_difficulty,
            now
          });
          target_difficulty = D.next_block_difficulty(
            block_period,
            difficulty_iterator(d_list.cbegin()),
            difficulty_iterator(d_list_bottom)
          );
          break;
        }
#endif
      }
      ++block.nNonce;
      if (block.nNonce == 0)
      {
        printf("NONCE WRAPPED, incrementing time\n");
        ++block.nTime;
      }
    }
    if (found) {
#ifndef DO_NOT_CALIBRATE_DIFFICULTY
      const auto now = coin::times::block::clock::now();
      const auto passed = now - start;
      if (passed < block_period / 2) {
        LOG() << "(genesis mining) "
          "target searching takes too short: "
              << duration_cast<seconds>(passed) 
              << " secs" << std::endl;
        d_list.push_front({
          d_list.front().height + 1,
          target_difficulty,
          now
        });
        target_difficulty = D.next_block_difficulty(
          block_period,
          difficulty_iterator(d_list.cbegin()),
          difficulty_iterator(d_list_bottom)
        );
        found = false;
      }
#if 0 // allow to disable higher difficulty in genesis
      else  if (passed > block_period + block_period / 2) {
        LOG() << "(genesis mining) "
                 "target searching takes too long: "
              << duration_cast<seconds>(passed) 
              << " secs" << std::endl;
        d_list.push_front({
          d_list.front().height + 1,
          target_difficulty,
          now
        });
        target_difficulty = D.next_block_difficulty(
          block_period,
          difficulty_iterator(d_list.cbegin()),
          difficulty_iterator(d_list_bottom)
        );
        found = false;
      }
#endif
#endif
    } 
  } while (!found);
  printf("block.nTime = %u \n", block.nTime);
  printf("block.nNonce = %u \n", block.nNonce);
  printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
}

} // genesis
