// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The generic scrypt implementations.
 *
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef SCRYPT_HPP
#define SCRYPT_HPP

#include <boost/detail/endian.hpp>
#include "constexpr_math.h"
#include "scrypt.h"

namespace meta {

//! The little-endian constexpr detector. C++11 sucks, use boost,
constexpr bool is_little_endian()
{
#ifdef BOOST_LITTLE_ENDIAN
  return true;
#else
  return false;
#endif
}

}

namespace scrypt {

void
PBKDF2_SHA256(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt,
    size_t saltlen, uint64_t c, uint8_t *buf, size_t dkLen);

inline uint32_t integerify(uint32_t a)
{
  return a;
}

#ifdef USE_SSE2
uint32_t integerify(__m128i a);
#endif

// The current SSE2 salsa20 implementation needs word
// rearrangement

template<unsigned r>
inline void rearrange_before(
  std::array<generic::SalsaBlock, 2*r>& x)
{
}

template<unsigned r>
inline void rearrange_after(std::array<generic::SalsaBlock, 2*r>& x)
{
}

#ifdef USE_SSE2
void rearrange_before(sse2::SalsaBlock& x);

template<unsigned r>
void rearrange_before(std::array<sse2::SalsaBlock, 2*r>& x)
{
  for (int k = 0; k < x.size(); k++)
    rearrange_before(x[k]);
}

void rearrange_after(sse2::SalsaBlock& x);

template<unsigned r>
void rearrange_after(std::array<sse2::SalsaBlock, 2*r>& x)
{
  for (int k = 0; k < x.size(); k++)
    rearrange_after(x[k]);
}
#endif

//! It is the Colin Percival's BlockMix implementation
template<
  unsigned r,
  class SalsaBlockT
>
std::array<SalsaBlockT, 2*r> block_mix(const std::array<SalsaBlockT, 2*r>& b)
{
  std::array<SalsaBlockT, 2*r> b1;

  // X <- B[2r-1]
  SalsaBlockT x(b[2*r - 1]);
  for (unsigned i = 0; i < r; i++) {
    // X <- H(X (+) B[2i])
    xor_salsa8(x, b[2*i]);
    // B'[i] <- X
    b1[i] = x;
    // X <- H(X (+) B[2i+1])
    xor_salsa8(x, b[2*i+1]);
    // B'[r+i] <- X
    b1[r+i] = x;
  }
  return b1;
}

//! It is the Colin Percival's ROMix implementation
template<
  uint32_t N,
  unsigned r,
  unsigned p,
  class SalsaBlockT,
  std::array<SalsaBlockT, 2*r> (*H)(const std::array<SalsaBlockT, 2*r>& b)
>
void romix
  (std::array<SalsaBlockT, 2*r>& x,    //< the input/output vector
   std::array<std::array<SalsaBlockT, 2*r>, N>& v   //< the work area
   )
{
  using namespace constexpr_math;

  // The current SSE2 salsa20 implementation needs word rearrangement
  rearrange_before<r>(x);

  for (uint32_t i = 0; i < N; i++) {
    v[i] = x;
    x = H(x);
  }
  for (uint32_t i = 0; i < N; i++) {
    static_assert(pow2x<log2x<N>::value>::value == N, "N must be pow of 2");
    // NB is N is pow 2 than we can integrify in mod 2^32-1
    // and then mod N
    const uint32_t j = integerify(x[2*r-1][0]) % N;
    x = H(x ^= v[j]);
  }

  rearrange_after<r>(x);
}

template<
  size_t N,   //< the number of cells to ROMix (the real number of
              //< used bytes is 1024*N*r/8 = 128*N*r.
              //< See (*) below. Due to this line N must be pow of 2
              //< and <= 2^32
  unsigned r, //< the size parameter to BlockMix (use r cells and
              //< salsa20/8 each, r = 2n, n >= 1, the BlockMix block
              //< size is 1024*r bits = 128*r bytes
  unsigned p    ,  //< the number of parallel processes, it is hardcoded as
                   //< 1 here, you need change the program for change
                   //< this parameter, do not try pass different value as the
                   //< template argument
  class SalsaBlockT,
  class Password, //< the input sequence - password
  class Salt, //< the input sequence - salt
  class Output //< the output sequence
>
void scrypt_256_sp_templ
  (
   const Password& password, 
   const Salt& salt,
         Output& output, 
         Scratchpad<N, r, p, SalsaBlockT>& scratchpad
  )
{
  std::array<std::array<SalsaBlockT, 2*r>, p> B;

  static_assert(sizeof(B) == 2 * r * p * sizeof(SalsaBlockT),
                "Invalid types definition");

  PBKDF2_SHA256(
    reinterpret_cast<const uint8_t*>(password.data()), 
    password.size(), 
    reinterpret_cast<const uint8_t*>(salt.data()),
    salt.size(), 
    1, 
    reinterpret_cast<uint8_t*>(B.data()),
    sizeof(B));

  static_assert(meta::is_little_endian(), 
    "The big endian version of scrypt is not implemented yet");

  static_assert(p == 1, 
    "The case p != 1 (multithreaded) is not implemented");
  romix<N, r, p, SalsaBlockT, block_mix<r, SalsaBlockT>>(B[0], scratchpad[0]);

  PBKDF2_SHA256
   (reinterpret_cast<const uint8_t*>(password.data()), 
    password.size(), 
    reinterpret_cast<uint8_t*>(B.data()), 
    sizeof(B), 
    1, 
    reinterpret_cast<uint8_t*>(output.data()), 
    output.size());
}

}

#endif
