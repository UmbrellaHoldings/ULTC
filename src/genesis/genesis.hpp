// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The genesis block abstraction.
 *
 * @author Sergei Lodyagin
 */

#ifndef BITCOIN_GENESIS_HPP
#define BITCOIN_GENESIS_HPP

#include <vector>
#include "btc_time.h"
#include "hash/hash.h"
#include "log.h"
#include "bignum.h"
#include "genesis/genesis.h"

namespace genesis {

template<
  const types::constexpr_string& phrase,
  int64_t reward,
  const types::constexpr_string& pubkey,
  unsigned time,
  unsigned nonce,
  unsigned difficulty,
  const uint256& hash
>
class the_block : public block
{
public:
  the_block() : block(CBlock())
  {
    using namespace coin::times::block;

    CTransaction txNew;
    txNew.vin.resize(1);
    txNew.vout.resize(1);

    assert(phrase.size() <= 89);
    txNew.vin[0].scriptSig = CScript() 
      << 486604799 << CBigNum(4) 
      << std::vector<unsigned char>(
           phrase.begin(),
           phrase.end()
         );
    txNew.vout[0].nValue = reward;
    txNew.vout[0].scriptPubKey = CScript() 
      << ParseHex(pubkey.c_str()) << OP_CHECKSIG;

    vtx.push_back(txNew);
    hashPrevBlock = 0;
    hashMerkleRoot = BuildMerkleTree();
    nVersion = 1;
    nBits    = difficulty;
    nTime    = time;
    nNonce   = nonce;
  }

  uint256 known_hash() const override
  { 
    return hash; 
  }
};

} // genesis

#endif

