// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#include <iostream>
#include <sstream>
#include <list>
#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"
#include "btc_time.h"
#include "log.h"
#include "block.h"
#include "genesis/genesis.h"
#include "algos/retarget.h"

class CWallet;
class CBlock;
class CBlockIndex;
class CKeyItem;
class CReserveKey;

class CAddress;
class CInv;
class CNode;
class CAuxPow;

struct CBlockIndexWorkComparator;

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE = 1000000;            // 1000KB block hard limit
/** Obsolete: maximum size for mined blocks */
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/4;     // 250KB  block soft limit
/** Default for -blockmaxsize, maximum size for mined blocks **/
static const unsigned int DEFAULT_BLOCK_MAX_SIZE = 250000;
/** Default for -blockprioritysize, maximum space for zero/low-fee transactions **/
static const unsigned int DEFAULT_BLOCK_PRIORITY_SIZE = 17000;
/** The maximum size for transactions we're willing to relay/mine */
static const unsigned int MAX_STANDARD_TX_SIZE = 100000;
/** The maximum allowed number of signature check operations in a block (network rule) */
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
/** The maximum number of orphan transactions kept in memory */
static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;
/** The maximum number of entries in an 'inv' protocol message */
static const unsigned int MAX_INV_SZ = 50000;
/** The maximum size of a blk?????.dat file (since 0.8) */
static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; // 128 MiB
/** The pre-allocation chunk size for blk?????.dat files (since 0.8) */
static const unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000; // 16 MiB
/** The pre-allocation chunk size for rev?????.dat files (since 0.8) */
static const unsigned int UNDOFILE_CHUNK_SIZE = 0x100000; // 1 MiB
/** Fake height value used in CCoins to signify they are only in the memory pool (since 0.8) */
static const unsigned int MEMPOOL_HEIGHT = 0x7FFFFFFF;
/** Dust Soft Limit, allowed with additional fee per output */
static const int64 DUST_SOFT_LIMIT = 100000; // 0.001 ULTC
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64 DUST_HARD_LIMIT = 1000;   // 0.00001 ULTC mininput
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;
/** Threshold for nLockTime: below this value it is interpreted as block number, otherwise as UNIX timestamp. */
static const unsigned int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC
/** Maximum number of script-checking threads allowed */
static const int MAX_SCRIPTCHECK_THREADS = 16;
#ifdef USE_UPNP
static const int fHaveUPnP = true;
#else
static const int fHaveUPnP = false;
#endif


extern CScript COINBASE_FLAGS;






extern CCriticalSection cs_main;
extern std::map<uint256, CBlockIndex*> mapBlockIndex;
extern std::set<CBlockIndex*, CBlockIndexWorkComparator> setBlockIndexValid;
extern CBlockIndex* pindexGenesisBlock;
extern int nBestHeight;
extern uint256 nBestChainWork;
extern uint256 nBestInvalidWork;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern unsigned int nTransactionsUpdated;
extern uint64 nLastBlockTx;
extern uint64 nLastBlockSize;
extern const std::string strMessageMagic;
extern double dHashesPerSec;
extern int64 nHPSTimerStart;
extern int64 nTimeBestReceived;
extern CCriticalSection cs_setpwalletRegistered;
extern std::set<CWallet*> setpwalletRegistered;
extern unsigned char pchMessageStart[4];
extern bool fImporting;
extern bool fReindex;
extern bool fBenchmark;
extern int nScriptCheckThreads;
extern bool fTxIndex;
extern unsigned int nCoinCacheSize;

// Settings
extern int64 nTransactionFee;
extern int64 nMinimumInputValue;

// Minimum disk space required - used in CheckDiskSpace()
static const uint64 nMinDiskSpace = 52428800;


class CReserveKey;
class CCoinsDB;
class CBlockTreeDB;
struct CDiskBlockPos;
class CCoins;
class CTxUndo;
class CCoinsView;
class CCoinsViewCache;
class CScriptCheck;
class CValidationState;

struct CBlockTemplate;

/** Register a wallet to receive updates from core */
void RegisterWallet(CWallet* pwalletIn);
/** Unregister a wallet from core */
void UnregisterWallet(CWallet* pwalletIn);
/** Push an updated transaction to all registered wallets */
void SyncWithWallets(const uint256 &hash, const CTransaction& tx, const CBlock* pblock = NULL, bool fUpdate = false);
/** Process an incoming block */
bool ProcessBlock(CValidationState &state, CNode* pfrom, CBlock* pblock, CDiskBlockPos *dbp = NULL);
/** Check whether enough disk space is available for an incoming block */
bool CheckDiskSpace(uint64 nAdditionalBytes = 0);
/** Open a block file (blk?????.dat) */
FILE* OpenBlockFile(const CDiskBlockPos &pos, bool fReadOnly = false);
/** Open an undo file (rev?????.dat) */
FILE* OpenUndoFile(const CDiskBlockPos &pos, bool fReadOnly = false);
/** Import blocks from an external file */
bool LoadExternalBlockFile(FILE* fileIn, CDiskBlockPos *dbp = NULL);
/** Initialize a new block tree database + block data on disk */
bool InitBlockIndex();
/** Load the block tree and coins database from disk */
bool LoadBlockIndex();
/** Unload database information */
void UnloadBlockIndex();
/** Verify consistency of the block and coin databases */
bool VerifyDB(int nCheckLevel, int nCheckDepth);
/** Print the loaded block tree */
void PrintBlockTree();
/** Find a block by height in the currently-connected chain */
CBlockIndex* FindBlockByHeight(int nHeight);
/** Process protocol messages received from a given node */
bool ProcessMessages(CNode* pfrom);
/** Send queued protocol messages to be sent to a give node */
bool SendMessages(CNode* pto, bool fSendTrickle);
/** Run an instance of the script checking thread */
void ThreadScriptCheck();
/** Run the miner threads */
void GenerateBitcoins(bool fGenerate, CWallet* pwallet);
/** Generate a new block, without valid proof-of-work */
CBlockTemplate* CreateNewBlock(
  const CScript& scriptPubKeyIn1,
  const CScript& scriptPubKeyIn2
);
CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey);
/** Modify the extranonce in a block */
void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);
void IncrementExtraNonceWithAux(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce, std::vector<unsigned char>& vchAux);
/** Do mining precalculation */
void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1);
/** Check mined block */
bool CheckWork(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey);

/** 
 * Check whether a block hash satisfies the proof-of-work
 * requirement. 
 */
template<class Block>
bool CheckProofOfWork(const Block& block)
{
  const bool is_auxpow_block = block.auxpow.get() != NULL;

#ifdef AUX_POW_STARTS_AT_BLOCK
  // to switch-off auxpow at all or before the specified
  // block
  const bool is_auxpow_allowed = 
    block.nHeight >= GetAuxPowStartBlock();
#else
  constexpr bool is_auxpow_allowed = true;
#endif

  if (is_auxpow_block && !is_auxpow_allowed)
    return error(
      "CheckProofOfWork() : AUX POW is not allowed "
      "at this block"
    );

  if (is_auxpow_allowed && !fTestNet 
#ifdef AUX_POW_STARTS_AT_BLOCK
      && block.nHeight != INT_MAX 
#endif
      && block.GetChainID() != pars::mm::GetOurChainID()
     )
    // Prevent same work from being submitted twice:
    // - this block must have our chain ID
    // - parent block must not have the same chain ID (see
    // - CAuxPow::Check)
    // - index of this chain in chain merkle tree must be
    // - pre-determined (see CAuxPow::Check)
    return error(
      "CheckProofOfWork() : block does not have "
      "our chain ID"
    );

  if (is_auxpow_allowed && is_auxpow_block &&
      !block.auxpow->Check(
        block.GetHash(), 
        block.GetChainID()
      )
     )
    return error(
      "CheckProofOfWork() : AUX POW is not valid"
    );

  // Check range
  if (!retarget::difficulty::instance().is_valid(block))
  return error(
    "CheckProofOfWork() : nBits below minimum work"
  );

  // Check proof of work matches claimed amount
  if ((is_auxpow_block 
         ? block.auxpow->GetParentBlockHash()
         : block.GetPoWHash()
       ) > CBigNum(block.nBits).getuint256()
      )
  return error(
    "CheckProofOfWork() : hash doesn't match nBits"
  );

  return true;
}

/** Get the number of active peers */
int GetNumBlocksOfPeers();
/** Check whether we are doing an initial block download (synchronizing from disk or network) */
bool IsInitialBlockDownload();
/** Format a string that describes several potential problems detected by the core */
std::string GetWarnings(std::string strFor);
/** Retrieve a transaction (from memory pool, or from disk, if possible) */
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock, bool fAllowSlow = false);
/** Connect/disconnect blocks until pindexNew is the new tip of the active block chain */
bool SetBestChain(CValidationState &state, CBlockIndex* pindexNew);
/** Find the best known block, and make it the tip of the block chain */
bool ConnectBestBlock(CValidationState &state);
/** Create a new block index entry for a given block hash */
CBlockIndex * InsertBlockIndex(uint256 hash);
/** Verify a signature */
bool VerifySignature(const CCoins& txFrom, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);
/** Abort with a message */
bool AbortNode(const std::string &msg);

bool GetWalletFile(CWallet* pwallet, std::string &strWalletFileOut);

struct CDiskTxPos : public CDiskBlockPos
{
  unsigned int nTxOffset; // after header

  IMPLEMENT_SERIALIZE(
    READWRITE(*(CDiskBlockPos*)this);
    READWRITE(VARINT(nTxOffset));
  )

  CDiskTxPos(const CDiskBlockPos &blockIn, unsigned int nTxOffsetIn) : CDiskBlockPos(blockIn.nFile, blockIn.nPos), nTxOffset(nTxOffsetIn) {
  }

  CDiskTxPos() {
    SetNull();
  }

  void SetNull() {
    CDiskBlockPos::SetNull();
    nTxOffset = 0;
  }
};


/** wrapper for CTxOut that provides a more compact serialization */
class CTxOutCompressor
{
private:
  CTxOut &txout;

public:
  static uint64 CompressAmount(uint64 nAmount);
  static uint64 DecompressAmount(uint64 nAmount);

  CTxOutCompressor(CTxOut &txoutIn) : txout(txoutIn) { }

  IMPLEMENT_SERIALIZE(({
    if (!fRead) {
      uint64 nVal = CompressAmount(txout.nValue);
      READWRITE(VARINT(nVal));
    } else {
      uint64 nVal = 0;
      READWRITE(VARINT(nVal));
      txout.nValue = DecompressAmount(nVal);
    }
    CScriptCompressor cscript(REF(txout.scriptPubKey));
    READWRITE(cscript);
  });)
};

/** Undo information for a CTxIn
 *
 *  Contains the prevout's CTxOut being spent, and if this was the
 *  last output of the affected transaction, its metadata as well
 *  (coinbase or not, height, transaction version)
 */
class CTxInUndo
{
public:
  CTxOut txout;     // the txout data before being spent
  bool fCoinBase;     // if the outpoint was the last unspent: whether it belonged to a coinbase
  unsigned int nHeight; // if the outpoint was the last unspent: its height
  int nVersion;     // if the outpoint was the last unspent: its version

  CTxInUndo() : txout(), fCoinBase(false), nHeight(0), nVersion(0) {}
  CTxInUndo(const CTxOut &txoutIn, bool fCoinBaseIn = false, unsigned int nHeightIn = 0, int nVersionIn = 0) : txout(txoutIn), fCoinBase(fCoinBaseIn), nHeight(nHeightIn), nVersion(nVersionIn) { }

  unsigned int GetSerializeSize(int nType, int nVersion) const {
    return ::GetSerializeSize(VARINT(nHeight*2+(fCoinBase ? 1 : 0)), nType, nVersion) +
         (nHeight > 0 ? ::GetSerializeSize(VARINT(this->nVersion), nType, nVersion) : 0) +
         ::GetSerializeSize(CTxOutCompressor(REF(txout)), nType, nVersion);
  }

  template<typename Stream>
  void Serialize(Stream &s, int nType, int nVersion) const {
    ::Serialize(s, VARINT(nHeight*2+(fCoinBase ? 1 : 0)), nType, nVersion);
    if (nHeight > 0)
      ::Serialize(s, VARINT(this->nVersion), nType, nVersion);
    ::Serialize(s, CTxOutCompressor(REF(txout)), nType, nVersion);
  }

  template<typename Stream>
  void Unserialize(Stream &s, int nType, int nVersion) {
    unsigned int nCode = 0;
    ::Unserialize(s, VARINT(nCode), nType, nVersion);
    nHeight = nCode / 2;
    fCoinBase = nCode & 1;
    if (nHeight > 0)
      ::Unserialize(s, VARINT(this->nVersion), nType, nVersion);
    ::Unserialize(s, REF(CTxOutCompressor(REF(txout))), nType, nVersion);
  }
};

/** Undo information for a CTransaction */
class CTxUndo
{
public:
  // undo information for all txins
  std::vector<CTxInUndo> vprevout;

  IMPLEMENT_SERIALIZE(
    READWRITE(vprevout);
  )
};

/** Undo information for a CBlock */
class CBlockUndo
{
public:
  std::vector<CTxUndo> vtxundo; // for all but the coinbase

  IMPLEMENT_SERIALIZE(
    READWRITE(vtxundo);
  )

  bool WriteToDisk(CDiskBlockPos &pos, const uint256 &hashBlock)
  {
    // Open history file to append
    CAutoFile fileout = CAutoFile(OpenUndoFile(pos), SER_DISK, CLIENT_VERSION);
    if (!fileout)
      return error("CBlockUndo::WriteToDisk() : OpenUndoFile failed");

    // Write index header
    unsigned int nSize = fileout.GetSerializeSize(*this);
    fileout << FLATDATA(pchMessageStart) << nSize;

    // Write undo data
    long fileOutPos = ftell(fileout);
    if (fileOutPos < 0)
      return error("CBlockUndo::WriteToDisk() : ftell failed");
    pos.nPos = (unsigned int)fileOutPos;
    fileout << *this;

    // calculate & write checksum
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << hashBlock;
    hasher << *this;
    fileout << hasher.GetHash();

    // Flush stdio buffers and commit to disk before returning
    fflush(fileout);
    if (!IsInitialBlockDownload())
      FileCommit(fileout);

    return true;
  }

  bool ReadFromDisk(const CDiskBlockPos &pos, const uint256 &hashBlock)
  {
    // Open history file to read
    CAutoFile filein = CAutoFile(OpenUndoFile(pos, true), SER_DISK, CLIENT_VERSION);
    if (!filein)
      return error("CBlockUndo::ReadFromDisk() : OpenBlockFile failed");

    // Read block
    uint256 hashChecksum;
    try {
      filein >> *this;
      filein >> hashChecksum;
    }
    catch (std::exception &e) {
      return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }

    // Verify checksum
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << hashBlock;
    hasher << *this;
    if (hashChecksum != hasher.GetHash())
      return error("CBlockUndo::ReadFromDisk() : checksum mismatch");

    return true;
  }
};

/** pruned version of CTransaction: only retains metadata and unspent transaction outputs
 *
 * Serialized format:
 * - VARINT(nVersion)
 * - VARINT(nCode)
 * - unspentness bitvector, for vout[2] and further; least significant byte first
 * - the non-spent CTxOuts (via CTxOutCompressor)
 * - VARINT(nHeight)
 *
 * The nCode value consists of:
 * - bit 1: IsCoinBase()
 * - bit 2: vout[0] is not spent
 * - bit 4: vout[1] is not spent
 * - The higher bits encode N, the number of non-zero bytes in the following bitvector.
 *   - In case both bit 2 and bit 4 are unset, they encode N-1, as there must be at
 *   least one non-spent output).
 *
 * Example: 0104835800816115944e077fe7c803cfa57f29b36bf87c1d358bb85e
 *      <><><--------------------------------------------><---->
 *      |  \          |               /
 *  version   code       vout[1]          height
 *
 *  - version = 1
 *  - code = 4 (vout[1] is not spent, and 0 non-zero bytes of bitvector follow)
 *  - unspentness bitvector: as 0 non-zero bytes follow, it has length 0
 *  - vout[1]: 835800816115944e077fe7c803cfa57f29b36bf87c1d35
 *         * 8358: compact amount representation for 60000000000 (600 BTC)
 *         * 00: special txout type pay-to-pubkey-hash
 *         * 816115944e077fe7c803cfa57f29b36bf87c1d35: address uint160
 *  - height = 203998
 *
 *
 * Example: 0109044086ef97d5790061b01caab50f1b8e9c50a5057eb43c2d9563a4eebbd123008c988f1a4a4de2161e0f50aac7f17e7f9555caa486af3b
 *      <><><--><--------------------------------------------------><----------------------------------------------><---->
 *     /  \   \           |                               |           /
 *  version  code  unspentness     vout[4]                           vout[16]       height
 *
 *  - version = 1
 *  - code = 9 (coinbase, neither vout[0] or vout[1] are unspent,
 *        2 (1, +1 because both bit 2 and bit 4 are unset) non-zero bitvector bytes follow)
 *  - unspentness bitvector: bits 2 (0x04) and 14 (0x4000) are set, so vout[2+2] and vout[14+2] are unspent
 *  - vout[4]: 86ef97d5790061b01caab50f1b8e9c50a5057eb43c2d9563a4ee
 *       * 86ef97d579: compact amount representation for 234925952 (2.35 BTC)
 *       * 00: special txout type pay-to-pubkey-hash
 *       * 61b01caab50f1b8e9c50a5057eb43c2d9563a4ee: address uint160
 *  - vout[16]: bbd123008c988f1a4a4de2161e0f50aac7f17e7f9555caa4
 *        * bbd123: compact amount representation for 110397 (0.001 BTC)
 *        * 00: special txout type pay-to-pubkey-hash
 *        * 8c988f1a4a4de2161e0f50aac7f17e7f9555caa4: address uint160
 *  - height = 120891
 */
class CCoins
{
public:
  // whether transaction is a coinbase
  bool fCoinBase;

  // unspent transaction outputs; spent outputs are .IsNull(); spent outputs at the end of the array are dropped
  std::vector<CTxOut> vout;

  // at which height this transaction was included in the active block chain
  int nHeight;

  // version of the CTransaction; accesses to this value should probably check for nHeight as well,
  // as new tx version will probably only be introduced at certain heights
  int nVersion;

  // construct a CCoins from a CTransaction, at a given height
  CCoins(const CTransaction &tx, int nHeightIn) : fCoinBase(tx.IsCoinBase()), vout(tx.vout), nHeight(nHeightIn), nVersion(tx.nVersion) { }

  // empty constructor
  CCoins() : fCoinBase(false), vout(0), nHeight(0), nVersion(0) { }

  // remove spent outputs at the end of vout
  void Cleanup() {
    while (vout.size() > 0 && vout.back().IsNull())
      vout.pop_back();
    if (vout.empty())
      std::vector<CTxOut>().swap(vout);
  }

  void swap(CCoins &to) {
    std::swap(to.fCoinBase, fCoinBase);
    to.vout.swap(vout);
    std::swap(to.nHeight, nHeight);
    std::swap(to.nVersion, nVersion);
  }

  // equality test
  friend bool operator==(const CCoins &a, const CCoins &b) {
     return a.fCoinBase == b.fCoinBase &&
        a.nHeight == b.nHeight &&
        a.nVersion == b.nVersion &&
        a.vout == b.vout;
  }
  friend bool operator!=(const CCoins &a, const CCoins &b) {
    return !(a == b);
  }

  // calculate number of bytes for the bitmask, and its number of non-zero bytes
  // each bit in the bitmask represents the availability of one output, but the
  // availabilities of the first two outputs are encoded separately
  void CalcMaskSize(unsigned int &nBytes, unsigned int &nNonzeroBytes) const {
    unsigned int nLastUsedByte = 0;
    for (unsigned int b = 0; 2+b*8 < vout.size(); b++) {
      bool fZero = true;
      for (unsigned int i = 0; i < 8 && 2+b*8+i < vout.size(); i++) {
        if (!vout[2+b*8+i].IsNull()) {
          fZero = false;
          continue;
        }
      }
      if (!fZero) {
        nLastUsedByte = b + 1;
        nNonzeroBytes++;
      }
    }
    nBytes += nLastUsedByte;
  }

  bool IsCoinBase() const {
    return fCoinBase;
  }

  unsigned int GetSerializeSize(int nType, int nVersion) const {
    unsigned int nSize = 0;
    unsigned int nMaskSize = 0, nMaskCode = 0;
    CalcMaskSize(nMaskSize, nMaskCode);
    bool fFirst = vout.size() > 0 && !vout[0].IsNull();
    bool fSecond = vout.size() > 1 && !vout[1].IsNull();
    assert(fFirst || fSecond || nMaskCode);
    unsigned int nCode = 8*(nMaskCode - (fFirst || fSecond ? 0 : 1)) + (fCoinBase ? 1 : 0) + (fFirst ? 2 : 0) + (fSecond ? 4 : 0);
    // version
    nSize += ::GetSerializeSize(VARINT(this->nVersion), nType, nVersion);
    // size of header code
    nSize += ::GetSerializeSize(VARINT(nCode), nType, nVersion);
    // spentness bitmask
    nSize += nMaskSize;
    // txouts themself
    for (unsigned int i = 0; i < vout.size(); i++)
      if (!vout[i].IsNull())
        nSize += ::GetSerializeSize(CTxOutCompressor(REF(vout[i])), nType, nVersion);
    // height
    nSize += ::GetSerializeSize(VARINT(nHeight), nType, nVersion);
    return nSize;
  }

  template<typename Stream>
  void Serialize(Stream &s, int nType, int nVersion) const {
    unsigned int nMaskSize = 0, nMaskCode = 0;
    CalcMaskSize(nMaskSize, nMaskCode);
    bool fFirst = vout.size() > 0 && !vout[0].IsNull();
    bool fSecond = vout.size() > 1 && !vout[1].IsNull();
    assert(fFirst || fSecond || nMaskCode);
    unsigned int nCode = 8*(nMaskCode - (fFirst || fSecond ? 0 : 1)) + (fCoinBase ? 1 : 0) + (fFirst ? 2 : 0) + (fSecond ? 4 : 0);
    // version
    ::Serialize(s, VARINT(this->nVersion), nType, nVersion);
    // header code
    ::Serialize(s, VARINT(nCode), nType, nVersion);
    // spentness bitmask
    for (unsigned int b = 0; b<nMaskSize; b++) {
      unsigned char chAvail = 0;
      for (unsigned int i = 0; i < 8 && 2+b*8+i < vout.size(); i++)
        if (!vout[2+b*8+i].IsNull())
          chAvail |= (1 << i);
      ::Serialize(s, chAvail, nType, nVersion);
    }
    // txouts themself
    for (unsigned int i = 0; i < vout.size(); i++) {
      if (!vout[i].IsNull())
        ::Serialize(s, CTxOutCompressor(REF(vout[i])), nType, nVersion);
    }
    // coinbase height
    ::Serialize(s, VARINT(nHeight), nType, nVersion);
  }

  template<typename Stream>
  void Unserialize(Stream &s, int nType, int nVersion) {
    unsigned int nCode = 0;
    // version
    ::Unserialize(s, VARINT(this->nVersion), nType, nVersion);
    // header code
    ::Unserialize(s, VARINT(nCode), nType, nVersion);
    fCoinBase = nCode & 1;
    std::vector<bool> vAvail(2, false);
    vAvail[0] = nCode & 2;
    vAvail[1] = nCode & 4;
    unsigned int nMaskCode = (nCode / 8) + ((nCode & 6) != 0 ? 0 : 1);
    // spentness bitmask
    while (nMaskCode > 0) {
      unsigned char chAvail = 0;
      ::Unserialize(s, chAvail, nType, nVersion);
      for (unsigned int p = 0; p < 8; p++) {
        bool f = (chAvail & (1 << p)) != 0;
        vAvail.push_back(f);
      }
      if (chAvail != 0)
        nMaskCode--;
    }
    // txouts themself
    vout.assign(vAvail.size(), CTxOut());
    for (unsigned int i = 0; i < vAvail.size(); i++) {
      if (vAvail[i])
        ::Unserialize(s, REF(CTxOutCompressor(vout[i])), nType, nVersion);
    }
    // coinbase height
    ::Unserialize(s, VARINT(nHeight), nType, nVersion);
    Cleanup();
  }

  // mark an outpoint spent, and construct undo information
  bool Spend(const COutPoint &out, CTxInUndo &undo) {
    if (out.n >= vout.size())
      return false;
    if (vout[out.n].IsNull())
      return false;
    undo = CTxInUndo(vout[out.n]);
    vout[out.n].SetNull();
    Cleanup();
    if (vout.size() == 0) {
      undo.nHeight = nHeight;
      undo.fCoinBase = fCoinBase;
      undo.nVersion = this->nVersion;
    }
    return true;
  }

  // mark a vout spent
  bool Spend(int nPos) {
    CTxInUndo undo;
    COutPoint out(0, nPos);
    return Spend(out, undo);
  }

  // check whether a particular output is still available
  bool IsAvailable(unsigned int nPos) const {
    return (nPos < vout.size() && !vout[nPos].IsNull());
  }

  // check whether the entire CCoins is spent
  // note that only !IsPruned() CCoins can be serialized
  bool IsPruned() const {
    BOOST_FOREACH(const CTxOut &out, vout)
      if (!out.IsNull())
        return false;
    return true;
  }
};

template <typename Stream>
int ReadWriteAuxPow(Stream& s, const boost::shared_ptr<CAuxPow>& auxpow, int nType, int nVersion, CSerActionSerialize ser_action);
  
template <typename Stream>
int ReadWriteAuxPow(Stream& s, boost::shared_ptr<CAuxPow>& auxpow, int nType, int nVersion, CSerActionUnserialize ser_action);
  
template <typename Stream>
int ReadWriteAuxPow(Stream& s, const boost::shared_ptr<CAuxPow>& auxpow, int nType, int nVersion, CSerActionGetSerializeSize ser_action);
  



/** Data structure that represents a partial merkle tree.
 *
 * It respresents a subset of the txid's of a known block, in a way that
 * allows recovery of the list of txid's and the merkle root, in an
 * authenticated way.
 *
 * The encoding works as follows: we traverse the tree in depth-first order,
 * storing a bit for each traversed node, signifying whether the node is the
 * parent of at least one matched leaf txid (or a matched txid itself). In
 * case we are at the leaf level, or this bit is 0, its merkle node hash is
 * stored, and its children are not explorer further. Otherwise, no hash is
 * stored, but we recurse into both (or the only) child branch. During
 * decoding, the same depth-first traversal is performed, consuming bits and
 * hashes as they written during encoding.
 *
 * The serialization is fixed and provides a hard guarantee about the
 * encoded size:
 *
 *   SIZE <= 10 + ceil(32.25*N)
 *
 * Where N represents the number of leaf nodes of the partial tree. N itself
 * is bounded by:
 *
 *   N <= total_transactions
 *   N <= 1 + matched_transactions*tree_height
 *
 * The serialization format:
 *  - uint32   total_transactions (4 bytes)
 *  - varint   number of hashes   (1-3 bytes)
 *  - uint256[]  hashes in depth-first order (<= 32*N bytes)
 *  - varint   number of bytes of flag bits (1-3 bytes)
 *  - byte[]   flag bits, packed per 8 in a byte, least significant bit first (<= 2*N-1 bits)
 * The size constraints follow from this.
 */
class CPartialMerkleTree
{
protected:
  // the total number of transactions in the block
  unsigned int nTransactions;

  // node-is-parent-of-matched-txid bits
  std::vector<bool> vBits;

  // txids and internal hashes
  std::vector<uint256> vHash;

  // flag set when encountering invalid data
  bool fBad;

  // helper function to efficiently calculate the number of nodes at given height in the merkle tree
  unsigned int CalcTreeWidth(int height) {
    return (nTransactions+(1 << height)-1) >> height;
  }

  // calculate the hash of a node in the merkle tree (at leaf level: the txid's themself)
  uint256 CalcHash(int height, unsigned int pos, const std::vector<uint256> &vTxid);

  // recursive function that traverses tree nodes, storing the data as bits and hashes
  void TraverseAndBuild(int height, unsigned int pos, const std::vector<uint256> &vTxid, const std::vector<bool> &vMatch);

  // recursive function that traverses tree nodes, consuming the bits and hashes produced by TraverseAndBuild.
  // it returns the hash of the respective node.
  uint256 TraverseAndExtract(int height, unsigned int pos, unsigned int &nBitsUsed, unsigned int &nHashUsed, std::vector<uint256> &vMatch);

public:

  // serialization implementation
  IMPLEMENT_SERIALIZE(
    READWRITE(nTransactions);
    READWRITE(vHash);
    std::vector<unsigned char> vBytes;
    if (fRead) {
      READWRITE(vBytes);
      CPartialMerkleTree &us = *(const_cast<CPartialMerkleTree*>(this));
      us.vBits.resize(vBytes.size() * 8);
      for (unsigned int p = 0; p < us.vBits.size(); p++)
        us.vBits[p] = (vBytes[p / 8] & (1 << (p % 8))) != 0;
      us.fBad = false;
    } else {
      vBytes.resize((vBits.size()+7)/8);
      for (unsigned int p = 0; p < vBits.size(); p++)
        vBytes[p / 8] |= vBits[p] << (p % 8);
      READWRITE(vBytes);
    }
  )

  // Construct a partial merkle tree from a list of transaction id's, and a mask that selects a subset of them
  CPartialMerkleTree(const std::vector<uint256> &vTxid, const std::vector<bool> &vMatch);

  CPartialMerkleTree();

  // extract the matching txid's represented by this partial merkle tree.
  // returns the merkle root, or 0 in case of failure
  uint256 ExtractMatches(std::vector<uint256> &vMatch);
};

class CBlockFileInfo
{
public:
  unsigned int nBlocks;    // number of blocks stored in file
  unsigned int nSize;    // number of used bytes of block file
  unsigned int nUndoSize;  // number of used bytes in the undo file
  unsigned int nHeightFirst; // lowest height of block in file
  unsigned int nHeightLast;  // highest height of block in file
  uint64 nTimeFirst;     // earliest time of block in file
  uint64 nTimeLast;      // latest time of block in file

  IMPLEMENT_SERIALIZE(
    READWRITE(VARINT(nBlocks));
    READWRITE(VARINT(nSize));
    READWRITE(VARINT(nUndoSize));
    READWRITE(VARINT(nHeightFirst));
    READWRITE(VARINT(nHeightLast));
    READWRITE(VARINT(nTimeFirst));
    READWRITE(VARINT(nTimeLast));
   )

   void SetNull() {
     nBlocks = 0;
     nSize = 0;
     nUndoSize = 0;
     nHeightFirst = 0;
     nHeightLast = 0;
     nTimeFirst = 0;
     nTimeLast = 0;
   }

   CBlockFileInfo() {
     SetNull();
   }

   std::string ToString() const {
     return strprintf("CBlockFileInfo(blocks=%u, size=%u, heights=%u...%u, time=%s...%s)", nBlocks, nSize, nHeightFirst, nHeightLast, DateTimeStrFormat("%Y-%m-%d", nTimeFirst).c_str(), DateTimeStrFormat("%Y-%m-%d", nTimeLast).c_str());
   }

   // update statistics (does not update nSize)
   void AddBlock(unsigned int nHeightIn, uint64 nTimeIn) {
     if (nBlocks==0 || nHeightFirst > nHeightIn)
       nHeightFirst = nHeightIn;
     if (nBlocks==0 || nTimeFirst > nTimeIn)
       nTimeFirst = nTimeIn;
     nBlocks++;
     if (nHeightIn > nHeightFirst)
       nHeightLast = nHeightIn;
     if (nTimeIn > nTimeLast)
       nTimeLast = nTimeIn;
   }
};

extern CCriticalSection cs_LastBlockFile;
extern CBlockFileInfo infoLastBlockFile;
extern int nLastBlockFile;

struct CBlockIndexWorkComparator
{
  bool operator()(CBlockIndex *pa, CBlockIndex *pb) {
    if (pa->nChainWork > pb->nChainWork) return false;
    if (pa->nChainWork < pb->nChainWork) return true;

    if (pa->GetBlockHash() < pb->GetBlockHash()) return false;
    if (pa->GetBlockHash() > pb->GetBlockHash()) return true;

    return false; // identical blocks
  }
};



/** Used to marshal pointers into hashes for db storage. */
class CDiskBlockIndex : public CBlockIndex
{
public:
  uint256 hashPrev;

  // if this is an aux work block
  boost::shared_ptr<CAuxPow> auxpow;

  CDiskBlockIndex() {
    hashPrev = 0;
    auxpow.reset();
  }

    explicit CDiskBlockIndex(CBlockIndex* pindex, boost::shared_ptr<CAuxPow> auxpow) : CBlockIndex(*pindex) {
    hashPrev = (pprev ? pprev->GetBlockHash() : 0);
        this->auxpow = auxpow;
  }

  IMPLEMENT_SERIALIZE
  (
        /* immutable stuff goes here, mutable stuff
         * has SERIALIZE functions in CBlockIndex */
    if (!(nType & SER_GETHASH))
      READWRITE(VARINT(nVersion));

    READWRITE(VARINT(nHeight));
    READWRITE(VARINT(nTx));

    // block header
    READWRITE(this->nVersion);
    READWRITE(hashPrev);
    READWRITE(hashMerkleRoot);
    READWRITE(nTime);
    READWRITE(nBits.compact);
    READWRITE(nNonce);
        ReadWriteAuxPow(s, auxpow, nType, this->nVersion, ser_action);
  )

    uint256 CalcBlockHash() const
  {
    CBlockHeader block;
    block.nVersion    = nVersion;
    block.hashPrevBlock   = hashPrev;
    block.hashMerkleRoot  = hashMerkleRoot;
    block.nTime       = nTime;
    block.nBits       = nBits;
    block.nNonce      = nNonce;
    return block.GetHash();
  }


    std::string ToString() const; // moved code to main.cpp
#if 0
  {
    std::string str = "CDiskBlockIndex(";
    str += CBlockIndex::ToString();
    str += strprintf("\n        hashBlock=%s, hashPrev=%s)",
      GetBlockHash().ToString().c_str(),
      hashPrev.ToString().c_str());
    return str;
  }
#endif

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};

/** Capture information about block/transaction validation */
class CValidationState {
private:
  enum mode_state {
    MODE_VALID,   // everything ok
    MODE_INVALID, // network rule violation (DoS value may be set)
    MODE_ERROR,   // run-time error
  } mode;
  int nDoS;
  bool corruptionPossible;
public:
  CValidationState() : mode(MODE_VALID), nDoS(0), corruptionPossible(false) {}
  bool DoS(int level, bool ret = false, bool corruptionIn = false) {
    if (mode == MODE_ERROR)
      return ret;
    nDoS += level;
    mode = MODE_INVALID;
    corruptionPossible = corruptionIn;
    return ret;
  }
  bool Invalid(bool ret = false) {
    return DoS(0, ret);
  }
  bool Error() {
    mode = MODE_ERROR;
    return false;
  }
  bool Abort(const std::string &msg) {
    AbortNode(msg);
    return Error();
  }
  bool IsValid() {
    return mode == MODE_VALID;
  }
  bool IsInvalid() {
    return mode == MODE_INVALID;
  }
  bool IsError() {
    return mode == MODE_ERROR;
  }
  bool IsInvalid(int &nDoSOut) {
    if (IsInvalid()) {
      nDoSOut = nDoS;
      return true;
    }
    return false;
  }
  bool CorruptionPossible() {
    return corruptionPossible;
  }
};







/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
class CBlockLocator
{
protected:
  std::vector<uint256> vHave;
public:

  CBlockLocator()
  {
  }

  explicit CBlockLocator(const CBlockIndex* pindex)
  {
    Set(pindex);
  }

  explicit CBlockLocator(uint256 hashBlock)
  {
    std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi != mapBlockIndex.end())
      Set((*mi).second);
  }

  CBlockLocator(const std::vector<uint256>& vHaveIn)
  {
    vHave = vHaveIn;
  }

  IMPLEMENT_SERIALIZE
  (
    if (!(nType & SER_GETHASH))
      READWRITE(nVersion);
    READWRITE(vHave);
  )

  void SetNull()
  {
    vHave.clear();
  }

  bool IsNull()
  {
    return vHave.empty();
  }

  void Set(const CBlockIndex* pindex)
  {
    vHave.clear();
    int nStep = 1;
    while (pindex)
    {
      vHave.push_back(pindex->GetBlockHash());

      // Exponentially larger steps back
      for (int i = 0; pindex && i < nStep; i++)
        pindex = pindex->pprev;
      if (vHave.size() > 10)
        nStep *= 2;
    }
    vHave.push_back(genesis::block::instance().known_hash());
  }

  int GetDistanceBack()
  {
    // Retrace how far back it was in the sender's branch
    int nDistance = 0;
    int nStep = 1;
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;
        if (pindex->IsInMainChain())
          return nDistance;
      }
      nDistance += nStep;
      if (nDistance > 10)
        nStep *= 2;
    }
    return nDistance;
  }

  CBlockIndex* GetBlockIndex()
  {
    // Find the first block the caller has in the main chain
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;
        if (pindex->IsInMainChain())
          return pindex;
      }
    }
    return pindexGenesisBlock;
  }

  uint256 GetBlockHash()
  {
    // Find the first block the caller has in the main chain
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;
        if (pindex->IsInMainChain())
          return hash;
      }
    }
    return genesis::block::instance().known_hash();
  }

  int GetHeight()
  {
    CBlockIndex* pindex = GetBlockIndex();
    if (!pindex)
      return 0;
    return pindex->nHeight;
  }
};








class CTxMemPool
{
public:
  mutable CCriticalSection cs;
  std::map<uint256, CTransaction> mapTx;
  std::map<COutPoint, CInPoint> mapNextTx;

  bool accept(CValidationState &state, CTransaction &tx, bool fCheckInputs, bool fLimitFree, bool* pfMissingInputs);
  bool addUnchecked(const uint256& hash, const CTransaction &tx);
  bool remove(const CTransaction &tx, bool fRecursive = false);
  bool removeConflicts(const CTransaction &tx);
  void clear();
  void queryHashes(std::vector<uint256>& vtxid);
  void pruneSpent(const uint256& hash, CCoins &coins);

  unsigned long size()
  {
    LOCK(cs);
    return mapTx.size();
  }

  bool exists(uint256 hash)
  {
    return (mapTx.count(hash) != 0);
  }

  CTransaction& lookup(uint256 hash)
  {
    return mapTx[hash];
  }
};

extern CTxMemPool mempool;

struct CCoinsStats
{
  int nHeight;
  uint256 hashBlock;
  uint64 nTransactions;
  uint64 nTransactionOutputs;
  uint64 nSerializedSize;
  uint256 hashSerialized;
  int64 nTotalAmount;

  CCoinsStats() : nHeight(0), hashBlock(0), nTransactions(0), nTransactionOutputs(0), nSerializedSize(0), hashSerialized(0), nTotalAmount(0) {}
};

/** Abstract view on the open txout dataset. */
class CCoinsView
{
public:
  // Retrieve the CCoins (unspent transaction outputs) for a given txid
  virtual bool GetCoins(const uint256 &txid, CCoins &coins);

  // Modify the CCoins for a given txid
  virtual bool SetCoins(const uint256 &txid, const CCoins &coins);

  // Just check whether we have data for a given txid.
  // This may (but cannot always) return true for fully spent transactions
  virtual bool HaveCoins(const uint256 &txid);

  // Retrieve the block index whose state this CCoinsView currently represents
  virtual CBlockIndex *GetBestBlock();

  // Modify the currently active block index
  virtual bool SetBestBlock(CBlockIndex *pindex);

  // Do a bulk modification (multiple SetCoins + one SetBestBlock)
  virtual bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, CBlockIndex *pindex);

  // Calculate statistics about the unspent transaction output set
  virtual bool GetStats(CCoinsStats &stats);

  // As we use CCoinsViews polymorphically, have a virtual destructor
  virtual ~CCoinsView() {}
};

/** CCoinsView backed by another CCoinsView */
class CCoinsViewBacked : public CCoinsView
{
protected:
  CCoinsView *base;

public:
  CCoinsViewBacked(CCoinsView &viewIn);
  bool GetCoins(const uint256 &txid, CCoins &coins);
  bool SetCoins(const uint256 &txid, const CCoins &coins);
  bool HaveCoins(const uint256 &txid);
  CBlockIndex *GetBestBlock();
  bool SetBestBlock(CBlockIndex *pindex);
  void SetBackend(CCoinsView &viewIn);
  bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, CBlockIndex *pindex);
  bool GetStats(CCoinsStats &stats);
};

/** CCoinsView that adds a memory cache for transactions to another CCoinsView */
class CCoinsViewCache : public CCoinsViewBacked
{
protected:
  CBlockIndex *pindexTip;
  std::map<uint256,CCoins> cacheCoins;

public:
  CCoinsViewCache(CCoinsView &baseIn, bool fDummy = false);

  // Standard CCoinsView methods
  bool GetCoins(const uint256 &txid, CCoins &coins);
  bool SetCoins(const uint256 &txid, const CCoins &coins);
  bool HaveCoins(const uint256 &txid);
  CBlockIndex *GetBestBlock();
  bool SetBestBlock(CBlockIndex *pindex);
  bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, CBlockIndex *pindex);

  // Return a modifiable reference to a CCoins. Check HaveCoins first.
  // Many methods explicitly require a CCoinsViewCache because of this method, to reduce
  // copying.
  CCoins &GetCoins(const uint256 &txid);

  // Push the modifications applied to this cache to its base.
  // Failure to call this method before destruction will cause the changes to be forgotten.
  bool Flush();

  // Calculate the size of the cache (in number of transactions)
  unsigned int GetCacheSize();

private:
  std::map<uint256,CCoins>::iterator FetchCoins(const uint256 &txid);
};

/** CCoinsView that brings transactions from a memorypool into view.
  It does not check for spendings by memory pool transactions. */
class CCoinsViewMemPool : public CCoinsViewBacked
{
protected:
  CTxMemPool &mempool;

public:
  CCoinsViewMemPool(CCoinsView &baseIn, CTxMemPool &mempoolIn);
  bool GetCoins(const uint256 &txid, CCoins &coins);
  bool HaveCoins(const uint256 &txid);
};

/** Global variable that points to the active CCoinsView (protected by cs_main) */
extern CCoinsViewCache *pcoinsTip;

/** Global variable that points to the active block tree (protected by cs_main) */
extern CBlockTreeDB *pblocktree;

struct CBlockTemplate
{
  CBlock block;
  std::vector<int64_t> vTxFees;
  std::vector<int64_t> vTxSigOps;
};

#if defined(_M_IX86) || defined(__i386__) || defined(__i386) || defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64)
extern unsigned int cpuid_edx;
#endif





/** Used to relay blocks as header + vector<merkle branch>
 * to filtered nodes.
 */
class CMerkleBlock
{
public:
  // Public only for unit testing
  CBlockHeader header;
  CPartialMerkleTree txn;

public:
  // Public only for unit testing and relay testing
  // (not relayed)
  std::vector<std::pair<unsigned int, uint256> > vMatchedTxn;

  // Create from a CBlock, filtering transactions according to filter
  // Note that this will call IsRelevantAndUpdate on the filter for each transaction,
  // thus the filter will likely be modified.
  CMerkleBlock(const CBlock& block, CBloomFilter& filter);

  IMPLEMENT_SERIALIZE
  (
    READWRITE(header);
    READWRITE(txn);
  )
};

#endif
