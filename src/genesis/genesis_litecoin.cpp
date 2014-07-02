// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The genesis block for Litecoin.
 */

#include "types/string.h"
#include "genesis.hpp"
#include "hash/hash.h"

using namespace types;

namespace genesis {

namespace litecoin {

const constexpr_string phrase = 
  "NY Times 05/Oct/2011 Steve Jobs, Apple’s Visionary, Dies at 56";
extern const constexpr_string phrase;

const constexpr_string pubkey =
  "040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3"
  "eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9";
extern const constexpr_string pubkey;

const uint256 hash = uint256(
  "0x12a765e31ffd4059bada1e25190f6e98c99d9714d334efa41a195a7e7e04bfe2"
);
extern const uint256 hash;  

const uint256 hash_testnet = uint256(
  "0xf5ae71e26c74beacc88382716aced69cddf3dffff24f384e1808905e0188f68f"
);
extern const uint256 hash_testnet;  

genesis::block* create()
{
  constexpr unsigned time = 1317972665;

  genesis::block* blk = new the_block<
    /* the coinbase */
    phrase,
    50 * COIN, // reward
    pubkey,
  
    // time
    time,
    //nonce
    2084524493,
    // difficuly
    0x1e0ffff0,
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
      "0x97ddfbbae6be97fd6cdf3e7ca13232a3afff2353e29ba"
      "dfab7f73011edd4ced9"
    ));

#if 0 // this part is used only on new genesis generation
    blk->mine();
#endif

  blk->print();

  assert(blk->known_hash() == real_hash);

  return blk;
}

genesis::block* create_testnet()
{
  constexpr unsigned time = 1317798646;

  genesis::block* blk = new the_block<
    /* the coinbase */
    phrase,
    50 * COIN, // reward
    pubkey,
  
    // time
    time,
    //nonce
    385270584,
    // difficuly
    0x1e0ffff0,
    hash_testnet
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

  return blk;
}

} // litecoin

} // genesis

