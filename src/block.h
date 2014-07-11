// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
//////////////////////////////////////////////////////////

/**
 * @file
 * A bitcoin block.
 *
 * @author Satoshi Nakamoto
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#ifndef BITCOIN_BLOCK_H
#define BITCOIN_BLOCK_H

#include <exception>
#include "bignum.h"
#include "log.h"
#include "script.h"
#include "btc_time.h"

namespace coin {

namespace except {

//! @exception Invalid block detected
struct invalid_block : virtual std::exception {};

//! @exception DoS attack detected
struct dos : virtual std::exception {};

//! @exception The block timestamp is invalid
struct invalid_timestamp : invalid_block, dos {};

//! @exception The block difficulty is invalid
struct invalid_difficulty : invalid_block, dos {};

} // except

} // coin

class CBlockIndex;
class CTransaction;
class CCoinsViewCache;
class CValidationState;
class CTxUndo;
class CDiskBlockPos;

enum GetMinFee_mode
{
  GMF_BLOCK,
  GMF_RELAY,
  GMF_SEND,
};

/** An inpoint - a combination of a transaction and an index n into its vin */
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

/** An outpoint - a combination of a transaction hash and an index n into its vout */
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
  int64 nValue;
  CScript scriptPubKey;

  CTxOut()
  {
    SetNull();
  }

  CTxOut(int64 nValueIn, CScript scriptPubKeyIn)
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
    nValue = -1;
    scriptPubKey.clear();
  }

  bool IsNull() const
  {
    return (nValue == -1);
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
  static int64 nMinTxFee;
  static int64 nMinRelayTxFee;
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
  int64 GetValueOut() const;

  /** Amount of bitcoins coming in to this transaction
    Note that lightweight clients may not know anything besides the hash of previous transactions,
    so may not be able to calculate this.

    @param[in] mapInputs  Map of previous transactions that have outputs we're spending
    @return  Sum of value of all inputs (scriptSigs)
   */
  int64 GetValueIn(CCoinsViewCache& mapInputs) const;

  static bool AllowFree(double dPriority)
  {
    // Large (in bytes) low-priority (new, small-coin) transactions
    // need a fee.
    return dPriority > COIN * 576 / 250;
  }

// Apply the effects of this transaction on the UTXO set represented by view
void UpdateCoins(const CTransaction& tx, CValidationState &state, CCoinsViewCache &inputs, CTxUndo &txundo, int nHeight, const uint256 &txhash);

  int64 GetMinFee(unsigned int nBlockSize=1, bool fAllowFree=true, enum GetMinFee_mode mode=GMF_BLOCK) const;

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

/** Nodes collect new transactions into a block, hash them
 * into a hash tree, and scan through nonce values to make
 * the block's hash satisfy proof-of-work requirements.
 * When they solve the proof-of-work, they broadcast the
 * block to everyone and the block is added to the block
 * chain.  The first transaction in the block is a special
 * one that creates a new coin owned by the creator of the
 * block.
 */
class CBlockHeader
{
public:
  // header
  static const int CURRENT_VERSION=2;
  int nVersion;
  uint256 hashPrevBlock;
  uint256 hashMerkleRoot;
  unsigned int nTime; //! seconds since Unix epoch
  compact_bignum_t nBits;
  unsigned int nNonce;
  boost::shared_ptr<CAuxPow> auxpow;

  CBlockHeader()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->nVersion);
    nVersion = this->nVersion;
    READWRITE(hashPrevBlock);
    READWRITE(hashMerkleRoot);
    READWRITE(nTime);
    READWRITE(nBits.compact);
    READWRITE(nNonce);

    nSerSize += ReadWriteAuxPow(s, auxpow, nType, nVersion, ser_action);
  )

  int GetChainID() const
  {
    return nVersion / BLOCK_VERSION_CHAIN_START;
  }

  uint256 GetPoWHash() const
  {
    uint256 thash;
    scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));
    return thash;
  }
	
  void SetAuxPow(CAuxPow* pow);

  void SetNull()
  {
    nVersion = CBlockHeader::CURRENT_VERSION | (GetOurChainID() * BLOCK_VERSION_CHAIN_START);
    hashPrevBlock = 0;
    hashMerkleRoot = 0;
    nTime = 0;
    nBits = 0;
    nNonce = 0;
  }

  bool IsNull() const
  {
    return (nBits == 0);
  }

  uint256 GetHash() const;

  int64 GetBlockTime() const
  {
    return (int64)nTime;
  }

  coin::times::block::time_point GetTimePoint() const;

  bool CheckProofOfWork(int nHeight) const;

  void UpdateTime(const CBlockIndex* pindexPrev);
};

class CBlock : public CBlockHeader
{
public:
  // network and disk
  std::vector<CTransaction> vtx;

  // memory only
  mutable std::vector<uint256> vMerkleTree;

  CBlock()
  {
    SetNull();
  }

  CBlock(const CBlockHeader &header)
  {
    SetNull();
    *((CBlockHeader*)this) = header;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(*(CBlockHeader*)this);
    READWRITE(vtx);
  )

  void SetNull()
  {
    CBlockHeader::SetNull();
    vtx.clear();
    vMerkleTree.clear();
  }

  //! Returns PoW hash of the block
  uint256 GetPoWHash() const;

  CBlockHeader GetBlockHeader() const
  {
    CBlockHeader block;
    block.nVersion     = nVersion;
    block.hashPrevBlock  = hashPrevBlock;
    block.hashMerkleRoot = hashMerkleRoot;
    block.nTime      = nTime;
    block.nBits      = nBits;
    block.nNonce     = nNonce;
    return block;
  }

  uint256 BuildMerkleTree() const;

  const uint256 &GetTxHash(unsigned int nIndex) const {
    assert(vMerkleTree.size() > 0); // BuildMerkleTree must have been called first
    assert(nIndex < vtx.size());
    return vMerkleTree[nIndex];
  }

  std::vector<uint256> GetMerkleBranch(int nIndex) const
  {
    if (vMerkleTree.empty())
      BuildMerkleTree();
    std::vector<uint256> vMerkleBranch;
    int j = 0;
    for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
    {
      int i = std::min(nIndex^1, nSize-1);
      vMerkleBranch.push_back(vMerkleTree[j+i]);
      nIndex >>= 1;
      j += nSize;
    }
    return vMerkleBranch;
  }

  static uint256 CheckMerkleBranch(
    uint256 hash, 
    const std::vector<uint256>& vMerkleBranch, 
    int nIndex
  );

  bool WriteToDisk(CDiskBlockPos &pos);

  bool ReadFromDisk(const CDiskBlockPos &pos);

  void print() const;

  /** Undo the effects of this block (with given index) on
   *  the UTXO set represented by coins.  In case pfClean
   *  is provided, operation will try to be tolerant about
   *  errors, and *pfClean will be true if no problems
   *  were found. Otherwise, the return value will be
   *  false in case of problems. Note that in any case,
   *  coins may be modified. */
  bool DisconnectBlock(CValidationState &state, CBlockIndex *pindex, CCoinsViewCache &coins, bool *pfClean = NULL);

  // Apply the effects of this block (with given index) on
  // the UTXO set represented by coins
  bool ConnectBlock(CValidationState &state, CBlockIndex *pindex, CCoinsViewCache &coins, bool fJustCheck=false);

  // Read a block from disk
  bool ReadFromDisk(const CBlockIndex* pindex);

  // Add this block to the block index, and if necessary,
  // switch the active block chain to this
  bool AddToBlockIndex(CValidationState &state, const CDiskBlockPos &pos);

  // Context-independent validity checks
  //  nHeight is needed to see if merged mining is allowed
  bool CheckBlock(CValidationState &state, int nHeight, bool fCheckPOW=true, bool fCheckMerkleRoot=true) const;

  // Store block on disk
  // if dbp is provided, the file is known to already
  // reside on disk
  bool AcceptBlock(CValidationState &state, CDiskBlockPos *dbp = NULL);
};

enum BlockStatus {
  BLOCK_VALID_UNKNOWN    =  0,
  BLOCK_VALID_HEADER     =  1, // parsed, version ok, hash satisfies claimed PoW, 1 <= vtx count <= max, timestamp not in future
  BLOCK_VALID_TREE     =  2, // parent found, difficulty matches, timestamp >= median previous, checkpoint
  BLOCK_VALID_TRANSACTIONS =  3, // only first tx is coinbase, 2 <= coinbase input script length <= 100, transactions valid, no duplicate txids, sigops, size, merkle root
  BLOCK_VALID_CHAIN    =  4, // outputs do not overspend inputs, no double spends, coinbase output ok, immature coinbase spends, BIP30
  BLOCK_VALID_SCRIPTS    =  5, // scripts/signatures ok
  BLOCK_VALID_MASK     =  7,

  BLOCK_HAVE_DATA      =  8, // full block available in blk*.dat
  BLOCK_HAVE_UNDO      =   16, // undo data available in rev*.dat
  BLOCK_HAVE_MASK      =   24,

  BLOCK_FAILED_VALID     =   32, // stage after last reached validness failed
  BLOCK_FAILED_CHILD     =   64, // descends from failed block
  BLOCK_FAILED_MASK    =   96
};

struct CDiskBlockPos
{
  int nFile;
  unsigned int nPos;

  IMPLEMENT_SERIALIZE(
    READWRITE(VARINT(nFile));
    READWRITE(VARINT(nPos));
  )

  CDiskBlockPos() {
    SetNull();
  }

  CDiskBlockPos(int nFileIn, unsigned int nPosIn) {
    nFile = nFileIn;
    nPos = nPosIn;
  }

  friend bool operator==(const CDiskBlockPos &a, const CDiskBlockPos &b) {
    return (a.nFile == b.nFile && a.nPos == b.nPos);
  }

  friend bool operator!=(const CDiskBlockPos &a, const CDiskBlockPos &b) {
    return !(a == b);
  }

  void SetNull() { nFile = -1; nPos = 0; }
  bool IsNull() const { return (nFile == -1); }
};

/** The block chain is a tree shaped structure starting
 * with the genesis block at the root, with each block
 * potentially having multiple candidates to be the next
 * block.  pprev and pnext link a path through the
 * main/longest chain.  A blockindex may have multiple
 * pprev pointing back to it, but pnext will only point
 * forward to the longest branch, or will be null if the
 * block is not part of the longest chain.
 */
class CBlockIndex
{
public:
  // pointer to the hash of the block, if any. memory is
  // owned by this CBlockIndex
  const uint256* phashBlock;

  // pointer to the index of the predecessor of this block
  CBlockIndex* pprev;

  // (memory only) pointer to the index of the *active*
  // successor of this block
  CBlockIndex* pnext;

  // height of the entry in the chain. The genesis block
  // has height 0
  int nHeight;

  // Which # file this block is stored in (blk?????.dat)
  int nFile;

  // Byte offset within blk?????.dat where this block's
  // data is stored
  unsigned int nDataPos;

  // Byte offset within rev?????.dat where this block's
  // undo data is stored
  unsigned int nUndoPos;

  // (memory only) Total amount of work (expected number
  // of hashes) in the chain up to and including this
  // block
  uint256 nChainWork;

  // Number of transactions in this block.  Note: in a
  // potential headers-first mode, this number cannot be
  // relied upon
  unsigned int nTx;

  // (memory only) Number of transactions in the chain up
  // to and including this block
  unsigned int nChainTx; // change to 64-bit type when
                         // necessary; won't happen before
                         // 2030

  // Verification status of this block. See enum BlockStatus
  unsigned int nStatus;

  // block header
  int nVersion;
  uint256 hashMerkleRoot;
  unsigned int nTime;
  compact_bignum_t nBits;
  unsigned int nNonce;


  CBlockIndex();
  CBlockIndex(CBlockHeader& block);

  IMPLEMENT_SERIALIZE
  (
    /* mutable stuff goes here, immutable stuff
     * has SERIALIZE functions in CDiskBlockIndex */
    if (!(nType & SER_GETHASH))
      READWRITE(VARINT(nVersion));
  
    READWRITE(VARINT(nStatus));
    if (nStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
      READWRITE(VARINT(nFile));
    if (nStatus & BLOCK_HAVE_DATA)
      READWRITE(VARINT(nDataPos));
    if (nStatus & BLOCK_HAVE_UNDO)
      READWRITE(VARINT(nUndoPos));
  )

  CDiskBlockPos GetBlockPos() const;
  CDiskBlockPos GetUndoPos() const;
  CBlockHeader GetBlockHeader() const;
  uint256 GetBlockHash() const;
  int64 GetBlockTime() const;
  coin::times::block::time_point GetTimePoint() const;
  CBigNum GetBlockWork() const;
  bool IsInMainChain() const;
  bool CheckIndex() const;

  enum { nMedianTimeSpan=11 };

  int64 GetMedianTimePast() const;
  int64 GetMedianTime() const;

  /**
   * Returns true if there are nRequired or more blocks of
   * minVersion or above in the last nToCheck blocks,
   * starting at pstart and going backwards.
   */
  static bool IsSuperMajority(int minVersion, const CBlockIndex* pstart,
                unsigned int nRequired, unsigned int nToCheck);

  std::string ToString() const;
  void print() const;
};


#endif

