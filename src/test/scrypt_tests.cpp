#include <algorithm>
#include <boost/test/unit_test.hpp>
#include "util.h"
#include "scrypt.hpp"

using namespace scrypt;
//using namespace scrypt::pars;

BOOST_AUTO_TEST_SUITE(scrypt_tests)

BOOST_AUTO_TEST_CASE(scrypt_hashtest)
{
    // 1. The 1024x1x1 80 in 32 out case

    {
      // Test Scrypt hash with known inputs against expected outputs
#define HASHCOUNT 5
      const char* inputhex[HASHCOUNT] = { "020000004c1271c211717198227392b029a64a7971931d351b387bb80db027f270411e398a07046f7d4a08dd815412a8712f874a7ebf0507e3878bd24e20a3b73fd750a667d2f451eac7471b00de6659", "0200000011503ee6a855e900c00cfdd98f5f55fffeaee9b6bf55bea9b852d9de2ce35828e204eef76acfd36949ae56d1fbe81c1ac9c0209e6331ad56414f9072506a77f8c6faf551eac7471b00389d01", "02000000a72c8a177f523946f42f22c3e86b8023221b4105e8007e59e81f6beb013e29aaf635295cb9ac966213fb56e046dc71df5b3f7f67ceaeab24038e743f883aff1aaafaf551eac7471b0166249b", "010000007824bc3a8a1b4628485eee3024abd8626721f7f870f8ad4d2f33a27155167f6a4009d1285049603888fe85a84b6c803a53305a8d497965a5e896e1a00568359589faf551eac7471b0065434e", "0200000050bfd4e4a307a8cb6ef4aef69abc5c0f2d579648bd80d7733e1ccc3fbc90ed664a7f74006cb11bde87785f229ecd366c2d4e44432832580e0608c579e4cb76f383f7f551eac7471b00c36982" };
      const char* expected[HASHCOUNT] = { "00000000002bef4107f882f6115e0b01f348d21195dacd3582aa2dabd7985806" , "00000000003a0d11bdd5eb634e08b7feddcfbbf228ed35d250daf19f1c88fc94", "00000000000b40f895f288e13244728a6c2d9d59d8aff29c65f8dd5114a8ca81", "00000000003007005891cd4923031e99d8e8d72f6e8e7edc6a86181897e105fe", "000000000018f0b426a4afc7130ccb47fa02af730d345b4fe7c7724d3800ec8c" };

      uint256 scrypthash;
      std::vector<unsigned char> inputbytes;
      Scratchpad<1024,1,1> scratchpad;
      for (int i = 0; i < HASHCOUNT; i++) {
        inputbytes = ParseHex(inputhex[i]);
#if defined(USE_SSE2)
        // Test SSE2 scrypt
        scrypt_256_sp_sse2_templ<1024, 1, 1>(inputbytes, scrypthash, scratchpad);
#endif
        // Test generic scrypt
        scrypt_256_sp_generic_templ<1024, 1, 1>
          (inputbytes, 
           scrypthash, 
           scratchpad);
        BOOST_CHECK_EQUAL(scrypthash.ToString().c_str(), expected[i]);
      }
    }

    // 2. The 16x1x1 64 out case
    // (the reference test from "STRONGER KEY DERIVATION VIA SEQUENTIAL
    // MEMORY-HARD FUNCTIONS", Colin Precival)

    {
      const std::string input = "";
      base_uint<512> expected;
      expected.SetHex("77d6576238657b203b19ca42c18a0497"
                      "f16b4844e3074ae8dfdffa3fede21442"
                      "fcd0069ded0948f8326a753a0fc81f17"
                      "e8d3e0fb2e0d3628cf35e20c38d18906");
      // It seams Colin gives it little-endian
      std::reverse(expected.begin(), expected.end());

      base_uint<512> scrypthash;
      Scratchpad<16, 1, 1> scratchpad;
#if defined(USE_SSE2)
      // Test SSE2 scrypt
      scrypt_256_sp_sse2_templ<16, 1, 1>
        (input, scrypthash, scratchpad);
#endif
        // Test generic scrypt
      scrypt_256_sp_generic_templ<16, 1, 1>
        (input, scrypthash, scratchpad);

      BOOST_CHECK_EQUAL(scrypthash.ToString().c_str(), expected.ToString().c_str());
    }

    // 3. The 16384x8x1 64 out case
    // (the reference test from "STRONGER KEY DERIVATION VIA SEQUENTIAL
    // MEMORY-HARD FUNCTIONS", Colin Precival)

    {
      // Test Scrypt hash with known inputs against expected outputs
      const std::string password = "pleaseletmein";
      const std::string salt = "SodiumChloride";

      assert(password.size() == 13);
      assert(salt.size() == 14);

      base_uint<512> expected;
      expected.SetHex("7023bdcb3afd7348461c06cd81fd38eb"
                      "fda8fbba904f8e3ea9b543f6545da1f2"
                      "d5432955613f0fcf62d49705242a9af9"
                      "e61e85dc0d651e40dfcf017b45575887");
      // It seams Colin gives it little-endian
      std::reverse(expected.begin(), expected.end());

      base_uint<512> scrypthash;
      Scratchpad<16384, 8, 1> scratchpad;
#if defined(USE_SSE2)
      // Test SSE2 scrypt
      scrypt_256_sp_sse2_templ<16384, 8, 1>
        (input, scrypthash, scratchpad);
#endif
        // Test generic scrypt
      scrypt_256_sp_generic_templ<16384, 8, 1>
        (password, 
         salt,
         scrypthash, 
         scratchpad);

      BOOST_CHECK_EQUAL(scrypthash.ToString().c_str(), expected.ToString().c_str());
    }
}

BOOST_AUTO_TEST_SUITE_END()
