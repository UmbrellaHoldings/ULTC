#include <iostream>
#include <cstdint>
#include <algorithm>

typedef int64_t int64;

int64 nChainStartTime = 1395198268; // Line: 2815

// yacoin: increasing Nfactor gradually
const unsigned char minNfactor = 10;
const unsigned char maxNfactor = 30;

int GetNfactor(int64 nTimestamp) {
    int l = 0;

    if (nTimestamp <= nChainStartTime)
        return minNfactor;

    int64 s = nTimestamp - nChainStartTime;
    while ((s >> 1) > 3) {
      l += 1;
      s >>= 1;
    }

    s &= 3;

    int n = (l * 158 + s * 28 - 2670) / 100;

    if (n < 0) n = 0;

    if (n > 255) {
        std::cout << "GetNfactor("
          << nTimestamp
          << ") - something wrong(n == "
          << n << ')' << std::endl;
    }

    unsigned char N = (unsigned char) n;
    //printf("GetNfactor: %d -> %d %d : %d / %d\n", nTimestamp - nChainStartTime, l, s, n, min(max(N, minNfactor), maxNfactor));

    return std::min(std::max(N, minNfactor), maxNfactor);
}

