/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  It is generic scrypt implementations.
*/

#include "scrypt.h"

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

  const size_t x_step = SALSA_BLOCK_SIZE / sizeof(x_el_t);
  const size_t last_x = (2 * r - 1) * x_step;

#if 0 // these are asserts for the case (N,r,p) = (1024,1,1)
  assert(x_step == 16);
  assert(x_size == 32);
  assert(blockmix_size == 128);
  assert(last_x == 16);
#endif

  for (int i = 0; i < N; i++) {
    // Vi <- X
    memcpy(&V[i * x_size], X, blockmix_size);

    // X <- H(X)
    xor_salsa8(&X[0], &X[last_x]);
    for (int l = 1; l <= 2 * r - 1; l++) {
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
    for (int l = 1; l <= 2 * r - 1; l++) {
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



