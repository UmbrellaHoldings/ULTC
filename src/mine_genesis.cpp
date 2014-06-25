/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  It will mine a new genesis block at the currency creation time.
  It was shared for everyone's profit by elbandi at https://bitcointalk.org/index.php?topic=391983.0
*/

#include <vector>
#include <memory>
#include <boost/thread.hpp>
#include "uint256.h"
#include "bignum.h"
#include "main.h"
#include "n_factor.h"
#include "hash/hash.h"
#include "log.h"

namespace genesis {

void block::mine()
{
  CBlock& block = *this;

  block.nNonce = 0;

  if (GetHash() != known_hash())
  {
    printf("Searching for genesis block...\n");
    // This will figure out a valid hash and Nonce if you're
    // creating a different genesis block:
    uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
    uint256 thash;
    auto H = hash::hasher::instance(block.GetTimePoint());
     
    loop
    {
      thash = H->hash(block);

      if (thash <= hashTarget)
        break;
      if ((block.nNonce & 0xFFF) == 0)
      {
        printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
      }
      ++block.nNonce;
      if (block.nNonce == 0)
      {
        printf("NONCE WRAPPED, incrementing time\n");
        ++block.nTime;
      }
    }
    printf("block.nTime = %u \n", block.nTime);
    printf("block.nNonce = %u \n", block.nNonce);
    printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
  }
}

} // genesis
