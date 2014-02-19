#ifndef SCRYPT_H
#define SCRYPT_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const int SCRYPT_N_PAR = 1024;
const int SCRYPT_R_PAR = 1;
const int SCRYPT_P_PAR = 1; // you must change scrypt_xxx algo to use
                            // threads if you want change this

const int SALSA_BLOCK_SIZE = 64; // in bytes

static const int SCRYPT_SCRATCHPAD_SIZE = 
  2* SALSA_BLOCK_SIZE * SCRYPT_N_PAR * SCRYPT_R_PAR * SCRYPT_P_PAR + 63;

void
PBKDF2_SHA256(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt,
    size_t saltlen, uint64_t c, uint8_t *buf, size_t dkLen);

static inline uint32_t le32dec(const void *pp)
{
        const uint8_t *p = (uint8_t const *)pp;
        return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
            ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

static inline void le32enc(void *pp, uint32_t x)
{
        uint8_t *p = (uint8_t *)pp;
        p[0] = x & 0xff;
        p[1] = (x >> 8) & 0xff;
        p[2] = (x >> 16) & 0xff;
        p[3] = (x >> 24) & 0xff;
}

void xor_salsa8(uint32_t B[16], const uint32_t Bx[16]);

template<
  size_t N,   // the number of cells to ROMix (the real number of
              // used bytes is 1024*N*r/8 = 128*N*r.
              // See (*) below. Due to this line N must be pow of 2
              // and <= 2^32
  unsigned r, // the size parameter to BlockMix (use r cells and
              // salsa20/8 each, r = 2n, n >= 1, the BlockMix block
              // size is 1024*r bits = 128*r bytes
  unsigned p /*= 1*/  // the number of parallel processes, it is hardcoded as
                  // 1 here, you need change the program for change
                  // this parameter, do not try pass different value as the
                  // template argument
>
void scrypt_256_sp_generic_templ
  (const char *input, 
   const size_t input_len,
   char *output, 
   const size_t output_len,
   char *scratchpad // 2 * r * SALSA_BLOCK_SIZE * p bytes
  )
{
  const size_t blockmix_size = SALSA_BLOCK_SIZE * 2 * r; // in bytes

  uint8_t B[blockmix_size * p];
  typedef uint32_t x_el_t;
  const size_t x_size = sizeof(B) / sizeof(x_el_t);
  x_el_t X[x_size]; 
  x_el_t *V;

  // align the buffer
  V = (uint32_t *)(((uintptr_t)(scratchpad) + 63) & ~ (uintptr_t)(63));
	
  PBKDF2_SHA256(
    (const uint8_t *)input, 
    input_len, 
    (const uint8_t *)input,
    input_len, 
    1, 
    B, 
    blockmix_size * p);

  // X <- B
  for (int k = 0; k < x_size; k++)
    X[k] = le32dec(&B[sizeof(X[0]) * k]);

  const size_t x_step = SALSA_BLOCK_SIZE / x_size;
  const size_t last_x = (2 * r - 1) * x_step;

  for (int i = 0; i < N; i++) {
    // Vi <- X
    memcpy(&V[i * x_size], X, blockmix_size);

    // X <- H(X)
    xor_salsa8(&X[0], &X[last_x]);
    for (int l = 1; l < 2 * r - 1; l++) {
      xor_salsa8(&X[l * x_step], &X[(l - 1) * x_step]);
    }
  }
  for (int i = 0; i < N; i++) {
    // j <- Integrify(X) mod N
    uint32_t j = x_size * (X[last_x] % N); // (*)
    // X <- X (+) Vj
    for (int k = 0; k < x_size; k++)
      X[k] ^= V[j + k];

    // X <- H(X)
    xor_salsa8(&X[0], &X[last_x]);
    for (int l = 1; l < 2 * r - 1; l++) {
      xor_salsa8(&X[l * x_step], &X[(l - 1) * x_step]);
    }
  }

  // B' <- X
  for (int k = 0; k < x_size; k++)
    le32enc(&B[sizeof(X[0]) * k], X[k]);

  PBKDF2_SHA256(
    (const uint8_t *)input, 
    input_len, 
    B, 
    blockmix_size * p, 
    1, 
    (uint8_t *)output, 
    output_len);
}

void scrypt_256_sp_generic
  (const char *input, char *output, char* scratchpad);

#if defined(USE_SSE2)
extern void scrypt_detect_sse2(unsigned int cpuid_edx);
//void scrypt_1024_1_1_256_sp_sse2(const char *input, char *output, char *scratchpad);
//extern void (*scrypt_1024_1_1_256_sp)(const char *input, char *output, char *scratchpad);
#endif

void scrypt_256(const char *input, char *output);

#endif
