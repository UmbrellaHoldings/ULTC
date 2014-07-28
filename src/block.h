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
//#include "auxpow.h"
#include "transaction.h"

class CAuxPow;

template <typename Stream>
int ReadWriteAuxPow(Stream& s, const boost::shared_ptr<CAuxPow>& auxpow, int nType, int nVersion, CSerActionGetSerializeSize ser_action);

enum
{
  // primary version
  BLOCK_VERSION_DEFAULT        = (1 << 0),

  // modifiers
  BLOCK_VERSION_AUXPOW         = (1 << 8),

  // bits allocated for chain ID
  BLOCK_VERSION_CHAIN_START    = (1 << 16),
  BLOCK_VERSION_CHAIN_END      = (1 << 30),
};

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
struct CDiskBlockPos;
class CAuxPow;

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

  int GetChainID() const;

  uint256 GetPoWHash() const;
	
  void SetAuxPow(CAuxPow* pow);

  void SetNull();

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

//  bool CheckProofOfWork(CBlock&) const;

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

