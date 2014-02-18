/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  It will mine a new genesis block at the currency creation time.
  It was shared for everyone's profit by elbandi at https://bitcointalk.org/index.php?topic=391983.0
*/

#include "uint256.h"
#include "bignum.h"
#include "main.h"
#include "scrypt.h"

extern uint256 hashGenesisBlock;

// If genesis block hash does not match, then generate new genesis hash.
void MineGenesisBlock(CBlock& block)
{
  block.nNonce = 0;

  if (block.GetHash() != hashGenesisBlock)
  {
    printf("Searching for genesis block...\n");
    // This will figure out a valid hash and Nonce if you're
    // creating a different genesis block:
    uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
    uint256 thash;
    char scratchpad[SCRYPT_SCRATCHPAD_SIZE];
     
    loop
    {
#if defined(USE_SSE2)
// Detection would work, but in cases where we KNOW it always has SSE2,
// it is faster to use directly than to use a function pointer or conditional.
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
// Always SSE2: x86_64 or Intel MacOS X
      scrypt_1024_1_1_256_sp_sse2(BEGIN(block.nVersion), BEGIN(thash), scratchpad);
#else
// Detect SSE2: 32bit x86 Linux or Windows
      scrypt_1024_1_1_256_sp(BEGIN(block.nVersion), BEGIN(thash), scratchpad);
#endif
#else
// Generic scrypt
      scrypt_1024_1_1_256_sp_generic(BEGIN(block.nVersion), BEGIN(thash), scratchpad);
#endif
      if (thash <= hashTarget)
        break;
      if ((block.nNonce & 0xFFF) == 0)
      {
        printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
      }
      ++block.nNonce;
      if (block.nNonce == 0)
      {
        printf("NONCE WRAPPED, incrementing time\n");
        ++block.nTime;
      }
    }
    printf("block.nTime = %u \n", block.nTime);
    printf("block.nNonce = %u \n", block.nNonce);
    printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
  }
}
