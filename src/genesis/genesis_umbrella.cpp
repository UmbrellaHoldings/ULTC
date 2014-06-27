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
  "0x00000001d8488b9dd2013a7ce1896b5ae94b7549881ebb5ce0538e1292b64d13"
);
extern const uint256 hash;  

const uint256 hash_testnet = uint256(
  "0x0000003bc489eb6cfb38dafd6bc9d390337bda36cb3aa04fdfc04e785c65fd48"
);
extern const uint256 hash_testnet;  

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
    137072358,
    // difficuly
    pars::min_difficulty_by_design.first,
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

genesis::block* create_testnet()
{
  constexpr unsigned time = 1403787088;

  genesis::block* blk = new the_block<
    /* the coinbase */
    phrase,
    10 * COIN, // reward
    pubkey,
  
    // time
    time,
    //nonce
    13571552,
    // difficuly
    pars::min_difficulty_by_design.second,
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
  const auto hash2 = hash::hasher::instance
    (coin::times::block::clock::from_nTime(time))
    -> hash(*blk);
  assert(real_hash == hash2);

  return blk;
}

} // umbrella

} // genesis
