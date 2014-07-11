// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * A bitcoin block.
 *
 * @author Satoshi Nakamoto
 * @author Sergei Lodyagin <serg@kogorta.dp.ua>
 */

#include "block.h"
#include "hash/hash.h"
#include "main.h"
#include "auxpow.h"
#include "txdb.h"

extern int nBestHeight;

uint256 CTransaction::GetHash() const
{
  return SerializeHash(*this);
}

int64 CTransaction::GetValueOut() const
{
  int64 nValueOut = 0;
  BOOST_FOREACH(const CTxOut& txout, vout)
  {
    nValueOut += txout.nValue;
    if (!MoneyRange(txout.nValue) || !MoneyRange(nValueOut))
      throw std::runtime_error(
        "CTransaction::GetValueOut() : value out of range"
      );
  }
  return nValueOut;
}

bool CTransaction::IsFinal(
  int nBlockHeight, 
  int64 nBlockTime
) const
{
  // Time based nLockTime implemented in 0.1.6
  if (nLockTime == 0)
    return true;
  if (nBlockHeight == 0)
    nBlockHeight = nBestHeight;
  if (nBlockTime == 0)
    nBlockTime = GetAdjustedTime();
  if ((int64)nLockTime < 
      ((int64)nLockTime < 
       LOCKTIME_THRESHOLD 
       ? (int64)nBlockHeight : nBlockTime
      )
      )
    return true;
  BOOST_FOREACH(const CTxIn& txin, vin)
    if (!txin.IsFinal())
      return false;
  return true;
}



uint256 CTxOut::GetHash() const
{
  return SerializeHash(*this);
}

int CBlockHeader::GetChainID() const
{
  return nVersion / BLOCK_VERSION_CHAIN_START;
}

uint256 CBlockHeader::GetHash() const
{
  return Hash(BEGIN(nVersion), END(nNonce));
}

void CBlockHeader::SetNull()
{
    nVersion = CBlockHeader::CURRENT_VERSION 
      | (pars::mm::GetOurChainID() * BLOCK_VERSION_CHAIN_START);
    hashPrevBlock = 0;
    hashMerkleRoot = 0;
    nTime = 0;
    nBits = 0;
    nNonce = 0;
}

uint256 CBlock::BuildMerkleTree() const
{
  vMerkleTree.clear();
  BOOST_FOREACH(const CTransaction& tx, vtx)
    vMerkleTree.push_back(tx.GetHash());
  int j = 0;
  for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
  {
    for (int i = 0; i < nSize; i += 2)
    {
      int i2 = std::min(i+1, nSize-1);
      vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
                                 BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
    }
    j += nSize;
  }
  return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
}

uint256 CBlock::CheckMerkleBranch(
  uint256 hash, 
  const std::vector<uint256>& vMerkleBranch, 
  int nIndex
)
{
  if (nIndex == -1)
    return 0;
  BOOST_FOREACH(const uint256& otherside, vMerkleBranch)
  {
    if (nIndex & 1)
      hash = Hash(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
    else
      hash = Hash(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
    nIndex >>= 1;
  }
  return hash;
}

bool CBlock::WriteToDisk(CDiskBlockPos &pos)
{
  // Open history file to append
  CAutoFile fileout = CAutoFile(OpenBlockFile(pos), SER_DISK, CLIENT_VERSION);
  if (!fileout)
    return error("CBlock::WriteToDisk() : OpenBlockFile failed");

  // Write index header
  unsigned int nSize = fileout.GetSerializeSize(*this);
  fileout << FLATDATA(pchMessageStart) << nSize;

  // Write block
  long fileOutPos = ftell(fileout);
  if (fileOutPos < 0)
    return error("CBlock::WriteToDisk() : ftell failed");
  pos.nPos = (unsigned int)fileOutPos;
  fileout << *this;

  // Flush stdio buffers and commit to disk before returning
  fflush(fileout);
  if (!IsInitialBlockDownload())
    FileCommit(fileout);

  return true;
}

bool CBlock::ReadFromDisk(const CDiskBlockPos &pos)
{
  SetNull();

  // Open history file to read
  CAutoFile filein = CAutoFile(OpenBlockFile(pos, true), SER_DISK, CLIENT_VERSION);
  if (!filein)
    return error("CBlock::ReadFromDisk() : OpenBlockFile failed");

  // Read block
  try {
    filein >> *this;
  }
  catch (std::exception &e) {
    return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
  }

  // Check the header
  if (!CheckProofOfWork(*this))
    return error("CBlock::ReadFromDisk() : errors in block header");

  return true;
}

void CBlock::print() const
{
  printf("CBlock(hash=%s, input=%s, PoW=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%" PRIszu ")\n",
         GetHash().ToString().c_str(),
         HexStr(BEGIN(nVersion),BEGIN(nVersion)+80,false).c_str(),
         GetPoWHash().ToString().c_str(),
         nVersion,
         hashPrevBlock.ToString().c_str(),
         hashMerkleRoot.ToString().c_str(),
         nTime, nBits.compact, nNonce,
         vtx.size());
  for (unsigned int i = 0; i < vtx.size(); i++)
  {
    printf("  ");
    vtx[i].print();
  }
  printf("  vMerkleTree: ");
  for (unsigned int i = 0; i < vMerkleTree.size(); i++)
    printf("%s ", vMerkleTree[i].ToString().c_str());
  printf("\n");
}

CBlockIndex::CBlockIndex()
{
  phashBlock = NULL;
  pprev = NULL;
  pnext = NULL;
  nHeight = 0;
  nFile = 0;
  nDataPos = 0;
  nUndoPos = 0;
  nChainWork = 0;
  nTx = 0;
  nChainTx = 0;
  nStatus = 0;

  nVersion     = 0;
  hashMerkleRoot = 0;
  nTime      = 0;
  nBits      = 0;
  nNonce     = 0;
}

CBlockIndex::CBlockIndex(CBlockHeader& block)
{
  phashBlock = NULL;
  pprev = NULL;
  pnext = NULL;
  nHeight = 0;
  nFile = 0;
  nDataPos = 0;
  nUndoPos = 0;
  nChainWork = 0;
  nTx = 0;
  nChainTx = 0;
  nStatus = 0;

  nVersion     = block.nVersion;
  hashMerkleRoot = block.hashMerkleRoot;
  nTime      = block.nTime;
  nBits      = block.nBits;
  nNonce     = block.nNonce;
}

CDiskBlockPos CBlockIndex::GetBlockPos() const 
{
  CDiskBlockPos ret;
  if (nStatus & BLOCK_HAVE_DATA) {
    ret.nFile = nFile;
    ret.nPos  = nDataPos;
  }
  return ret;
}

CDiskBlockPos CBlockIndex::GetUndoPos() const 
{
  CDiskBlockPos ret;
  if (nStatus & BLOCK_HAVE_UNDO) {
    ret.nFile = nFile;
    ret.nPos  = nUndoPos;
  }
  return ret;
}

CBlockHeader CBlockIndex::GetBlockHeader() const
{
  CBlockHeader block;

  if (nVersion & BLOCK_VERSION_AUXPOW) {
    CDiskBlockIndex diskblockindex;
    // auxpow is not in memory, load CDiskBlockHeader
    // from database to get it

    pblocktree->ReadDiskBlockIndex(*phashBlock, diskblockindex);
    block.auxpow = diskblockindex.auxpow;
  }

  block.nVersion     = nVersion;
  if (pprev)
    block.hashPrevBlock = pprev->GetBlockHash();
  block.hashMerkleRoot = hashMerkleRoot;
  block.nTime      = nTime;
  block.nBits      = nBits;
  block.nNonce     = nNonce;
  return block;
}

uint256 CBlockIndex::GetBlockHash() const
{
  return *phashBlock;
}

int64 CBlockIndex::GetBlockTime() const
{
  return (int64)nTime;
}

CBigNum CBlockIndex::GetBlockWork() const
{
  CBigNum bnTarget;
  bnTarget.SetCompact(nBits);
  if (bnTarget <= 0)
    return 0;
  return (CBigNum(1)<<256) / (bnTarget+1);
}

bool CBlockIndex::IsInMainChain() const
{
  return (pnext || this == pindexBest);
}

bool CBlockIndex::CheckIndex() const
{
  /** Scrypt is used for block proof-of-work, but for
   *  purposes of performance the index internally uses
   *  sha256.  This check was considered unneccessary
   *  given the other safeguards like the genesis and
   *  checkpoints. */
  return true; // return CheckProofOfWork(GetBlockHash(),
               // nBits);
}

int64 CBlockIndex::GetMedianTimePast() const
{
  int64 pmedian[nMedianTimeSpan];
  int64* pbegin = &pmedian[nMedianTimeSpan];
  int64* pend = &pmedian[nMedianTimeSpan];

  const CBlockIndex* pindex = this;
  for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
    *(--pbegin) = pindex->GetBlockTime();

  std::sort(pbegin, pend);
  return pbegin[(pend - pbegin)/2];
}

int64 CBlockIndex::GetMedianTime() const
{
  const CBlockIndex* pindex = this;
  for (int i = 0; i < nMedianTimeSpan/2; i++)
  {
    if (!pindex->pnext)
      return GetBlockTime();
    pindex = pindex->pnext;
  }
  return pindex->GetMedianTimePast();
}

std::string CBlockIndex::ToString() const
{
  return strprintf("CBlockIndex(pprev=%p, pnext=%p, nHeight=%d, merkle=%s, hashBlock=%s)",
                   pprev, pnext, nHeight,
                   hashMerkleRoot.ToString().c_str(),
                   GetBlockHash().ToString().c_str());
}

void CBlockIndex::print() const
{
  printf("%s\n", ToString().c_str());
}
