// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The genesis block for Umbrella-{BTC,LTC}.
 *
 * @author Anastasia Kurbatova
 * @author Sergei Lodyagin
 */

#include "types/string.h"
#include "genesis.hpp"
#include "hash/hash.h"

using namespace types;

namespace genesis {

namespace umbrella {

const constexpr_string phrase = 
  "If the sky falls, we shall catch larks!";
extern const constexpr_string phrase;

const constexpr_string pubkey =
  "0487e11b7e3b8803bef76182af8faa8566191aa37e301e0f8"
  "bc01ca668a265c5a11c2d95a689c8432e6aae6e5d0be182c9"
  "db9c2fa6494e49b0e464c1b87da7f9be";
extern const constexpr_string pubkey;

const uint256 hash = uint256(
  "0x00002e91fb9d6c7e9f5f6debf68e1e993bc9ffaaeaa9efe"
  "358e7fc0be2dfa83b"
);
extern const uint256 hash;  

genesis::block* create()
{
  constexpr unsigned time = 1403698060;

  genesis::block* blk = new the_block<
    /* the coinbase */
    phrase,
    10 * COIN, // reward
    pubkey,
  
    // time
    time,
    //nonce
    463861,
    // difficuly
    0x1f003fff,
    hash
  >();

  const uint256 real_hash = blk->GetHash();

  // debug print
  LOG() << "genesis sha256d: " << real_hash << std::endl;
  LOG() << "genesis merkle root: " << blk->hashMerkleRoot
        << std::endl;

  assert(
    blk->hashMerkleRoot == 
    uint256(
      "0x77a2709a91515ab610c6543228bdd45b2c56a72f9cbaa"
      "137a1a292c59afbdb35"
    ));

#if 0 // this part is used only on new genesis generation
    blk->mine();
#endif

  blk->print();

  assert(blk->known_hash() == real_hash);
  const auto hash2 = hash::hasher::instance
    (coin::times::block::clock::from_nTime(time))
    -> hash(*blk);
  assert(real_hash == hash2);

  return blk;
}

} // umbrella

} // genesis
