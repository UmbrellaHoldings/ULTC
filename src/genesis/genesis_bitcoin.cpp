// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The genesis block for bitcoin.
 *
 * @author Satoshi Nakamoto
 * @author Sergei Lodyagin
 */

#include "types/string.h"
#include "genesis.hpp"
#include "hash/hash.h"

using namespace types;

namespace genesis {

namespace bitcoin {

const constexpr_string phrase = 
  "The Times 03/Jan/2009 Chancellor on brink "
  "of second bailout for banks";
extern const constexpr_string phrase;

const constexpr_string pubkey =
  "04678afdb0fe5548271967f1a67130b7105cd6a828e03909a"
  "67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de"
  "5c384df7ba0b8d578a4c702b6bf11d5f";
extern const constexpr_string pubkey;

const uint256 hash = uint256(
  "0x000000000019d6689c085ae165831e934ff763ae46a"
  "2a6c172b3f1b60a8ce26f"
);
extern const uint256 hash;  

genesis::block* create()
{
  constexpr unsigned time = 1231006505;

  genesis::block* blk = new the_block<
    /* the coinbase */
    phrase,
    50 * COIN, // reward
    pubkey,
  
    // time
    time,
    //nonce
    2083236893,
    // difficuly
    0x1d00ffff,
    hash
  >();

  const uint256 real_hash = blk->GetHash();

  // debug print
  LOG() << "genesis sha256d: " << real_hash << std::endl;
  LOG() << "genesis merkle root: " << blk->hashMerkleRoot
        << std::endl;

  assert(blk->hashMerkleRoot == uint256("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

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

} // bitcoin

} // genesis
