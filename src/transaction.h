// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
//////////////////////////////////////////////////////////

/**
 * @file
 * A bitcoin transaction.
 *
 * @author Satoshi Nakamoto
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_TRANSACTION_H
#define BITCOIN_TRANSACTION_H

#include "log.h"

class CCoinsViewCache;
class CValidationState;
class CTxUndo;
class CBlock;
class CBlockIndex;

enum GetMinFee_mode
{
  GMF_BLOCK,
  GMF_RELAY,
  GMF_SEND,
};

/** An inpoint - a combination of a transaction and an
* index n into its vin */
class CInPoint
{
public:
  CTransaction* ptx;
  unsigned int n;

  CInPoint() { SetNull(); }
  CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
  void SetNull() { ptx = NULL; n = (unsigned int) -1; }
  bool IsNull() const { return (ptx == NULL && n == (unsigned int) -1); }
};

class COutPoint;
std::ostream&
operator<<(std::ostream& out, const COutPoint& op);

/** An outpoint - a combination of a transaction hash and
* an index n into its vout */
class COutPoint
{
public:
  uint256 hash;
  unsigned int n;

  COutPoint() { SetNull(); }
  COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
  IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
  void SetNull() { hash = 0; n = (unsigned int) -1; }
  bool IsNull() const { return (hash == 0 && n == (unsigned int) -1); }

  friend bool operator<(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
  }

  friend bool operator==(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash == b.hash && a.n == b.n);
  }

  friend bool operator!=(const COutPoint& a, const COutPoint& b)
  {
    return !(a == b);
  }

  //! @deprecated
  std::string ToString() const
  {
    std::ostringstream out;
    out << *this;
    return out.str();
  }

  //! @deprecated
  void print() const
  {
    LOG() << *this << std::flush;
  }
};

class CTxIn;
std::ostream& 
operator<<(std::ostream& out, const CTxIn& t);

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
  COutPoint prevout;
  CScript scriptSig;
  unsigned int nSequence;

  CTxIn()
  {
    nSequence = std::numeric_limits<unsigned int>::max();
  }

  explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(prevout);
    READWRITE(scriptSig);
    READWRITE(nSequence);
  )

  bool IsFinal() const
  {
    return (nSequence == std::numeric_limits<unsigned int>::max());
  }

  friend bool operator==(const CTxIn& a, const CTxIn& b)
  {
    return (a.prevout   == b.prevout &&
        a.scriptSig == b.scriptSig &&
        a.nSequence == b.nSequence);
  }

  friend bool operator!=(const CTxIn& a, const CTxIn& b)
  {
    return !(a == b);
  }

  //! @deprecated
  std::string ToString() const
  {
    std::ostringstream out;
    out << *this;
    return out.str();
  }

  //! @deprecated
  void print() const
  {
    LOG() << *this << std::flush;
  }
};

std::ostream& operator<<(
  std::ostream& out, 
  const std::vector<CTxIn>& trs
);

class CTxOut;
std::ostream& 
operator<<(std::ostream& out, const CTxOut& t);

/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
  serialize::money_t nValue;
  CScript scriptPubKey;

  CTxOut()
  {
    SetNull();
  }

  CTxOut(money_t nValueIn, CScript scriptPubKeyIn)
  {
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(nValue);
    READWRITE(scriptPubKey);
  )

  void SetNull()
  {
    nValue = -1.0_satoshi;
    scriptPubKey.clear();
  }

  bool IsNull() const
  {
    return (nValue == -1.0_satoshi);
  }

  uint256 GetHash() const;

  friend bool operator==(const CTxOut& a, const CTxOut& b)
  {
    return (a.nValue     == b.nValue &&
        a.scriptPubKey == b.scriptPubKey);
  }

  friend bool operator!=(const CTxOut& a, const CTxOut& b)
  {
    return !(a == b);
  }

  bool IsDust() const;

  //! @deprecated
  std::string ToString() const
  {
    std::ostringstream out;
    out << *this;
    return out.str();
  }

  //! @deprecated
  void print() const
  {
    LOG() << *this << std::flush;
  }
};

std::ostream& operator<<(
  std::ostream& out, 
  const std::vector<CTxOut>& trs
);

/** The basic transaction that is broadcasted on the
 * network and contained in blocks. A transaction can
 * contain multiple inputs and outputs.
 */
class CTransaction
{
public:
  static money_t nMinTxFee;
  static money_t nMinRelayTxFee;
  static const int CURRENT_VERSION=1;
  int nVersion;
  std::vector<CTxIn> vin;
  std::vector<CTxOut> vout;
  unsigned int nLockTime;

  CTransaction()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->nVersion);
    nVersion = this->nVersion;
    READWRITE(vin);
    READWRITE(vout);
    READWRITE(nLockTime);
  )

  void SetNull()
  {
    nVersion = CTransaction::CURRENT_VERSION;
    vin.clear();
    vout.clear();
    nLockTime = 0;
  }

  bool IsNull() const
  {
    return (vin.empty() && vout.empty());
  }

  uint256 GetHash() const;

  bool IsFinal(int nBlockHeight=0, int64 nBlockTime=0) const;

  bool IsNewerThan(const CTransaction& old) const
  {
    if (vin.size() != old.vin.size())
      return false;
    for (unsigned int i = 0; i < vin.size(); i++)
      if (vin[i].prevout != old.vin[i].prevout)
        return false;

    bool fNewer = false;
    unsigned int nLowest = std::numeric_limits<unsigned int>::max();
    for (unsigned int i = 0; i < vin.size(); i++)
    {
      if (vin[i].nSequence != old.vin[i].nSequence)
      {
        if (vin[i].nSequence <= nLowest)
        {
          fNewer = false;
          nLowest = vin[i].nSequence;
        }
        if (old.vin[i].nSequence < nLowest)
        {
          fNewer = true;
          nLowest = old.vin[i].nSequence;
        }
      }
    }
    return fNewer;
  }

  bool IsCoinBase() const
  {
    return (vin.size() == 1 && vin[0].prevout.IsNull());
  }

  /** Check for standard transaction types
    @return True if all outputs (scriptPubKeys) use only standard transaction forms
  */
  bool IsStandard(std::string& strReason) const;
  bool IsStandard() const
  {
    std::string strReason;
    return IsStandard(strReason);
  }

  /** Check for standard transaction types
    @param[in] mapInputs  Map of previous transactions that have outputs we're spending
    @return True if all inputs (scriptSigs) use only standard transaction forms
  */
  bool AreInputsStandard(CCoinsViewCache& mapInputs) const;

  /** Count ECDSA signature operations the old-fashioned (pre-0.6) way
    @return number of sigops this transaction's outputs will produce when spent
  */
  unsigned int GetLegacySigOpCount() const;

  /** Count ECDSA signature operations in pay-to-script-hash inputs.

    @param[in] mapInputs  Map of previous transactions that have outputs we're spending
    @return maximum number of sigops required to validate this transaction's inputs
   */
  unsigned int GetP2SHSigOpCount(CCoinsViewCache& mapInputs) const;

  /** Amount of bitcoins spent by this transaction.
    @return sum of all outputs (note: does not include fees)
   */
  money_t GetValueOut() const;

  /** Amount of bitcoins coming in to this transaction
    Note that lightweight clients may not know anything besides the hash of previous transactions,
    so may not be able to calculate this.

    @param[in] mapInputs  Map of previous transactions that have outputs we're spending
    @return  Sum of value of all inputs (scriptSigs)
   */
  money_t GetValueIn(CCoinsViewCache& mapInputs) const;

  static bool AllowFree(double dPriority)
  {
    // Large (in bytes) low-priority (new, small-coin) transactions
    // need a fee.
    return dPriority > COIN * 576 / 250;
  }

// Apply the effects of this transaction on the UTXO set represented by view
void UpdateCoins(const CTransaction& tx, CValidationState &state, CCoinsViewCache &inputs, CTxUndo &txundo, int nHeight, const uint256 &txhash);

  money_t GetMinFee(
    unsigned int nBlockSize=1, 
    bool fAllowFree=true, 
    enum GetMinFee_mode mode=GMF_BLOCK
  ) const;

  friend bool operator==(const CTransaction& a, const CTransaction& b)
  {
    return (a.nVersion  == b.nVersion &&
        a.vin     == b.vin &&
        a.vout    == b.vout &&
        a.nLockTime == b.nLockTime);
  }

  friend bool operator!=(const CTransaction& a, const CTransaction& b)
  {
    return !(a == b);
  }


  std::string ToString() const
  {
    std::string str;
    str += strprintf("CTransaction(hash=%s, ver=%d, vin.size=%" PRIszu ", vout.size=%" PRIszu ", nLockTime=%u)\n",
      GetHash().ToString().c_str(),
      nVersion,
      vin.size(),
      vout.size(),
      nLockTime);
    for (unsigned int i = 0; i < vin.size(); i++)
      str += "  " + vin[i].ToString() + "\n";
    for (unsigned int i = 0; i < vout.size(); i++)
      str += "  " + vout[i].ToString() + "\n";
    return str;
  }

  void print() const
  {
    printf("%s", ToString().c_str());
  }


  // Check whether all prevouts of this transaction are present in the UTXO set represented by view
  bool HaveInputs(CCoinsViewCache &view) const;

  // Check whether all inputs of this transaction are valid (no double spends, scripts & sigs, amounts)
  // This does not modify the UTXO set. If pvChecks is not NULL, script checks are pushed onto it
  // instead of being performed inline.
  bool CheckInputs(CValidationState &state, CCoinsViewCache &view, bool fScriptChecks = true,
           unsigned int flags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC,
           std::vector<CScriptCheck> *pvChecks = NULL) const;

  // Apply the effects of this transaction on the UTXO set represented by view
  void UpdateCoins(CValidationState &state, CCoinsViewCache &view, CTxUndo &txundo, int nHeight, const uint256 &txhash) const;

  // Context-independent validity checks
  bool CheckTransaction(CValidationState &state) const;

  // Try to accept this transaction into the memory pool
  bool AcceptToMemoryPool(CValidationState &state, bool fCheckInputs=true, bool fLimitFree = true, bool* pfMissingInputs=NULL);

protected:
  static const CTxOut &GetOutputFor(const CTxIn& input, CCoinsViewCache& mapInputs);
};

std::ostream& 
operator<<(std::ostream& out, const CTransaction& tr);

/** A transaction with a merkle branch linking it to the
* block chain. */
class CMerkleTx : public CTransaction
{
public:
  uint256 hashBlock;
  std::vector<uint256> vMerkleBranch;
  int nIndex;

  // memory only
  mutable bool fMerkleVerified;


  CMerkleTx()
  {
    Init();
  }

  CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
  {
    Init();
  }

  void Init()
  {
    hashBlock = 0;
    nIndex = -1;
    fMerkleVerified = false;
  }


  IMPLEMENT_SERIALIZE
  (
    nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
    nVersion = this->nVersion;
    READWRITE(hashBlock);
    READWRITE(vMerkleBranch);
    READWRITE(nIndex);
  )


  int SetMerkleBranch(const CBlock* pblock=NULL);
  int GetDepthInMainChain(CBlockIndex* &pindexRet) const;
  int GetDepthInMainChain() const { CBlockIndex *pindexRet; return GetDepthInMainChain(pindexRet); }
  bool IsInMainChain() const { return GetDepthInMainChain() > 0; }
  int GetBlocksToMaturity() const;
  bool AcceptToMemoryPool(bool fCheckInputs=true, bool fLimitFree=true);
};


#endif
