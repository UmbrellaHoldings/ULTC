// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The generic scrypt implementations.
 *
 * @author Warren Togami <wtogami@gmail.com>
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef SCRYPT_H
#define SCRYPT_H

#include <array>
#include <cstdint>
#include <emmintrin.h>
#include "uint256.h"

namespace scrypt {

//! One block for salsa20/8
template<class Base>
class SalsaBlock : public std::array<Base, 64/sizeof(Base)>
{
public:
  using Parent = std::array<Base, 64/sizeof(Base)>;

  SalsaBlock operator ^= (const SalsaBlock& b)
  {
    for(typename Parent::size_type i = 0; i < Parent::size(); i++)
      (*this)[i] ^= b[i];
    return *this;
  }
};

//! The dynamic-allocated memory for scrypt
template<uint32_t N, unsigned r, unsigned p, class SalsaBlockT>
using Scratchpad alignas(64) = 
  std::array<std::array<std::array<SalsaBlockT, 2*r>, N>, p>;

namespace generic {

using SalsaBlock = scrypt::SalsaBlock<uint32_t>;
static_assert(sizeof(SalsaBlock) == 512/8, "Invalid types definition");

}

namespace sse2 {

using SalsaBlock = scrypt::SalsaBlock<__m128i>;
static_assert(sizeof(SalsaBlock) == 512/8, "Invalid types definition");

}

//! It's bit XOR for arrays
template<class T, size_t n>
std::array<T, n>& operator ^= (std::array<T, n>& a, const std::array<T, n>& b)
{
  for(size_t i = 0; i < n; i++)
    a[i] ^= b[i];
  return a;
}

namespace usdollarcoin {

namespace pars {

// The USDollarCoin specific scrypt parameters
// The amount of used memory is 2 * r * n * p *
// sizeof(SalsaBlock) = 128*r*n*p

constexpr size_t mem_amount = 128 * 1024 * 1024; 

constexpr unsigned r = 8;
constexpr unsigned p = 1; // you must change scrypt_xxx algo to use
                          // threads if you want change
                          // this

static_assert(sizeof(generic::SalsaBlock) == sizeof(sse2::SalsaBlock),
              "Invalid types definition");
constexpr size_t n = mem_amount / (2 * r * p * sizeof(generic::SalsaBlock));

//! The output length of scrypt hash in bytes (32)
//constexpr unsigned output_len = 256 / 8; 

}

template<class SalsaBlockT>
using Scratchpad = scrypt::Scratchpad<pars::n, pars::r, pars::p, SalsaBlockT>;

}

void xor_salsa8(generic::SalsaBlock& B, const generic::SalsaBlock& Bx);
void xor_salsa8(sse2::SalsaBlock& B, const sse2::SalsaBlock& Bx);

#if defined(USE_SSE2)
extern void scrypt_detect_sse2(unsigned int cpuid_edx);
#endif

#if defined(USE_SSE2)
#  define SSE2_OR_GENERIC sse2
#else
#  define SSE2_OR_GENERIC generic
#endif

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
  );

}

#endif
