// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

/**
 * @file
 * The genesis block abstraction.
 *
 * @author Sergei Lodyagin
 */

#ifndef BITCOIN_GENESIS_H
#define BITCOIN_GENESIS_H

#include "block.h"
#include "pars.h"

namespace genesis {

class block : public CBlock
{
public:
  static block& instance()
  {
    static boost::once_flag of = BOOST_ONCE_INIT;
    static block* instance = nullptr;
    boost::call_once([]()
    { 
      instance = pars::create_genesis_block(); 
    }, of);
    assert(instance);
    return *instance;
  }

  virtual uint256 known_hash() const 
  { 
    return 0;
  }

  //! If known_hash() and GetHash() do not match, then
  //! generate new genesis hash by incrementing (nTime,
  //! nNonce).
  void mine();

protected:
  block(const CBlock& blk) : CBlock(blk) {}

};

} // genesis

#endif
