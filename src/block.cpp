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


uint256 CBlockHeader::GetHash() const
{
  return Hash(BEGIN(nVersion), END(nNonce));
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
