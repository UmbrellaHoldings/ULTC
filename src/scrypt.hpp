// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The generic scrypt implementations.
 *
 * @author Warren Togami <wtogami@gmail.com>
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef SCRYPT_HPP
#define SCRYPT_HPP

#include "scrypt.h"
#include <boost/detail/endian.hpp>

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

//! It is the Colin Percival's BlockMix implementation
template<unsigned r>
std::array<SalsaBlock, 2*r> block_mix(const std::array<SalsaBlock, 2*r>& b)
{
  std::array<SalsaBlock, 2*r> b1;

  // X <- B[2r-1]
  SalsaBlock x(b[2*r - 1]);
  for (int i = 0; i < r; i++) {
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
  std::array<SalsaBlock, 2*r> (*H)(const std::array<SalsaBlock, 2*r>& b)
>
void romix
  (std::array<SalsaBlock, 2*r>& x,    //< the input/output vector
   std::array<std::array<SalsaBlock, 2*r>, N>& v   //< the work area
   )
{
  for (int i = 0; i < N; i++) {
    v[i] = x;
    x = H(x);
  }
  for (int i = 0; i < N; i++) {
    const uint32_t j = x[2*r-1][0] % N;
    x = H(x ^= v[j]);
  }
}

template<
  size_t N,   //< the number of cells to ROMix (the real number of
              //< used bytes is 1024*N*r/8 = 128*N*r.
              //< See (*) below. Due to this line N must be pow of 2
              //< and <= 2^32
  unsigned r, //< the size parameter to BlockMix (use r cells and
              //< salsa20/8 each, r = 2n, n >= 1, the BlockMix block
              //< size is 1024*r bits = 128*r bytes
  unsigned p = 1,  //< the number of parallel processes, it is hardcoded as
                   //< 1 here, you need change the program for change
                   //< this parameter, do not try pass different value as the
                   //< template argument
  class Password, //< the input sequence - password
  class Salt, //< the input sequence - salt
  class Output //< the output sequence
>
void scrypt_256_sp_generic_templ
  (
   const Password& password, 
   const Salt& salt,
         Output& output, 
         Scratchpad<N, r, p>& scratchpad
  )
{
  std::array<std::array<SalsaBlock, 2*r>, p> B;

  static_assert(sizeof(B) == 2 * r * p * sizeof(SalsaBlock),
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
  romix<N, r, p, block_mix<r>>(B[0], scratchpad[0]);

  PBKDF2_SHA256
   (reinterpret_cast<const uint8_t*>(password.data()), 
    password.size(), 
    reinterpret_cast<uint8_t*>(B.data()), 
    sizeof(B), 
    1, 
    reinterpret_cast<uint8_t*>(output.data()), 
    output.size());
}

template<
  size_t N,   //< the number of cells to ROMix (the real number of
              //< used bytes is 1024*N*r/8 = 128*N*r.
              //< See (*) below. Due to this line N must be pow of 2
              //< and <= 2^32
  unsigned r, //< the size parameter to BlockMix (use r cells and
              //< salsa20/8 each, r = 2n, n >= 1, the BlockMix block
              //< size is 1024*r bits = 128*r bytes
  unsigned p = 1,  //< the number of parallel processes, it is hardcoded as
                   //< 1 here, you need change the program for change
                   //< this parameter, do not try pass different value as the
                   //< template argument
  class Input, //< the input sequence
  class Output //< the output sequence
>
void scrypt_256_sp_generic_templ
  (
   const Input& input, 
         Output& output, 
         Scratchpad<N, r, p>& scratchpad
  )
{
  scrypt_256_sp_generic_templ<N, r, p, Input, Input, Output>
    (input, input, output, scratchpad);
}

}

#endif
