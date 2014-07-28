/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2014 Sergei Lodyagin 
 
  The utility for generate bitcoin pub/private key pairs.
*/

#include <fstream>
#include "util.h"
#include "key.h"
#include "base58.h"

using namespace std;

bool fTestNet = false;

int main(int argc, char* argv[])
{
  static const char* fpub_name = "pub.key";
  static const char* fpriv_name = "priv.key";

  RandAddSeedPerfmon();
  CKey secret;
  secret.MakeNewKey(false);
  const CPubKey pub = secret.GetPubKey();
  const CPrivKey priv = secret.GetPrivKey();

#if 0
  {
    ofstream fpub(fpub_name);
    fpub << HexStr(pub.begin(), pub.end());
  }

  {
    ofstream fpriv(fpriv_name);
    fpriv << HexStr(priv.begin(), priv.end());
  }
#else
  CKey secretC; // compressed 
  secretC.Set(secret.begin(), secret.end(), true);
  std::cout << CBitcoinSecret(secret).ToString()  << std::endl;
  std::cout << CBitcoinSecret(secretC).ToString()  << std::endl;
  std::cout << CBitcoinAddress(pub.GetID()).ToString() << std::endl;
  std::cout << CBitcoinAddress(secretC.GetPubKey().GetID()).ToString() << std::endl;
#endif
}
