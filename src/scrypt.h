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
#include "uint256.h"

namespace scrypt {

//! One block for salsa20/8
class SalsaBlock : public std::array<uint32_t, 64/sizeof(uint32_t)>
{
public:
  SalsaBlock operator ^= (const SalsaBlock& b)
  {
    for(size_type i = 0; i < size(); i++)
      (*this)[i] ^= b[i];
    return *this;
  }
};
static_assert(sizeof(SalsaBlock) == 512/8, "Invalid types definition");

//! It's bit XOR for arrays
template<class T, size_t n>
std::array<T, n>& operator ^= (std::array<T, n>& a, const std::array<T, n>& b)
{
  for(size_t i = 0; i < n; i++)
    a[i] ^= b[i];
  return a;
}

//! The dynamic-allocated memory for scrypt
template<uint32_t N, unsigned r, unsigned p>
using Scratchpad alignas(64) = 
  std::array<std::array<std::array<SalsaBlock, 2*r>, N>, p>;

namespace usdollarcoin {

namespace pars {

// The USDollarCoin specific scrypt parameters

constexpr size_t n = 1024;
constexpr unsigned r = 1;
constexpr unsigned p = 1; // you must change scrypt_xxx algo to use
                          // threads if you want change this

//! The output length of scrypt hash in bytes (32)
constexpr unsigned output_len = 256 / 8; 

}

using Scratchpad = scrypt::Scratchpad<pars::n, pars::r, pars::p>;

void scrypt_256_sp_generic
  (const char* input, 
   uint256& output, 
   Scratchpad& scratchpad);

void scrypt_256(const char *input, uint256& output);

}

void
PBKDF2_SHA256(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt,
    size_t saltlen, uint64_t c, uint8_t *buf, size_t dkLen);

void xor_salsa8(SalsaBlock& B, const SalsaBlock& Bx);

#if defined(USE_SSE2)
extern void scrypt_detect_sse2(unsigned int cpuid_edx);
//void scrypt_1024_1_1_256_sp_sse2(const char *input, char *output, char *scratchpad);
//extern void (*scrypt_1024_1_1_256_sp)(const char *input, char *output, char *scratchpad);
#endif

}

#endif
