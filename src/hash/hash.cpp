#include "hash/hash.h"
#include "hash/scrypt.hpp"
#include "n_factor.h"
#include "main.h"

namespace hash {

namespace scrypt {

scratchpad_ptr get_scratchpad(n_factor_t n_factor);

uint256 hash(
  const CBlock& blk,
  n_factor_t n_factor,
  scratchpad_ptr scr
);

}

template<pars::hash_fun>
class hasher_impl;

template<>
class hasher_impl<pars::hash_fun::brittcoin_scrypt> 
  : public hasher
{
public:
  hasher_impl( 
    coin::times::block::time_point block_time
  )
    : n_factor(GetNfactor(block_time)),
      scratchpad(scrypt::get_scratchpad(n_factor))
  {}

  uint256 hash(const CBlock& blk) override
  {
    return scrypt::hash(blk, n_factor, scratchpad.get());
  }

protected:
  const n_factor_t n_factor;
  std::unique_ptr<scratchpad_base> scratchpad;
};

template<>
class hasher_impl<pars::hash_fun::scrypt> 
  : public hasher
{
public:
  hasher_impl( 
    coin::times::block::time_point block_time
  )
    : scratchpad(scrypt::scratchpad<1024, 1, 1>::allocate())
  {}

  uint256 hash(const CBlock& blk) override
  {
    const std::string in(BEGIN(blk.nVersion), 80);
    uint256 hash;

    auto* sp = dynamic_cast<scrypt::scratchpad<1024, 1, 1>*>(scratchpad);
    assert(sp);
    scrypt::scrypt_256_sp_templ<1024, 1, 1>(
      in, in, hash,
      sp->pad
    );
    return hash;
  }

protected:
  scratchpad_base* scratchpad;
};

template<>
class hasher_impl<pars::hash_fun::sha256d> 
  : public hasher
{
public:
  hasher_impl( 
    coin::times::block::time_point block_time
  )
  {}

  uint256 hash(const CBlock& blk) override
  {
    return blk.GetHash();
  }
};

std::shared_ptr<hasher> hasher::instance(
  coin::times::block::time_point block_time
)
{
  return std::make_shared
    <hasher_impl<pars::hash_function>>
      (block_time);
}

} // hash

inline uint32_t ROTL32 ( uint32_t x, int8_t r )
{
    return (x << r) | (x >> (32 - r));
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash)
{
    // The following is MurmurHash3 (x86_32), see http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp
    uint32_t h1 = nHashSeed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    const int nblocks = vDataToHash.size() / 4;

    //----------
    // body
    const uint32_t * blocks = (const uint32_t *)(&vDataToHash[0] + nblocks*4);

    for(int i = -nblocks; i; i++)
    {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = ROTL32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1,13); 
        h1 = h1*5+0xe6546b64;
    }

    //----------
    // tail
    const uint8_t * tail = (const uint8_t*)(&vDataToHash[0] + nblocks*4);

    uint32_t k1 = 0;

    switch(vDataToHash.size() & 3)
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
            k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization
    h1 ^= vDataToHash.size();
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}
