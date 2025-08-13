// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/consensus.h>
#include <primitives/block.h>
#include "primitives/blockcache.h"
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>
#include <crypto/minotaurx/minotaur.h>
#include <validation.h>
#include <util.h>
#include <sync.h>
#include <pulsar.h>



uint256 CBlockHeader::GetSHA256Hash() const
{
    return SerializeHash(*this);
}

uint256 CBlockHeader::ComputeBlockHash() const
{
    switch (GetPoWType()) {
    case POW_TYPE_CURVEHASH: {
        uint256 powHash;
        GetCurvehashHash(this, &powHash);
        return powHash;
        break;
    }
    case POW_TYPE_MINOTAURX: {
        return Minotaur(BEGIN(nVersion), END(nNonce), true);
        break;
    }
    default: // Don't crash the client on invalid blockType, just return a bad hash
        LogPrintf("---: ComputePOWHash HIGH HASH ERROR - PowType: %s \n", GetPoWType());
        return HIGH_HASH;
    }

}

uint256 CBlockHeader::GetBlockHash(bool readCache) const
{
    LOCK(cs_pow);
    CBlockCache& cache(CBlockCache::Instance());

    uint256 headerHash = GetSHA256Hash();
    uint256 powHash;
    bool found = false;

    if (readCache) {
        found = cache.get(headerHash, powHash);
    }

    if (!found || cache.IsValidate()) {
        uint256 powHash2 = ComputeBlockHash();
        if (found && powHash2 != powHash) {
            LogPrintf("BlockCache failure: headerHash: %s, from cache: %s, computed: %s, correcting\n", headerHash.ToString(), powHash.ToString(), powHash2.ToString());
        }
        powHash = powHash2;
        cache.erase(headerHash); // If it exists, replace it.
        cache.insert(headerHash, powHash2);
        //LogPrintf("BlockCache New %s: headerHash: %s, powHash: %s \n", GetPoWType(), headerHash.ToString(), powHash.ToString());
    }
    return powHash;
}

uint256 CBlockHeader::GetHash(bool readCache) const
{
    uint256 powHash;
    switch (GetPoWType()) {
    case POW_TYPE_CURVEHASH: {
        return SerializeHash(*this);
        break;
    }
    case POW_TYPE_MINOTAURX: {
        powHash = GetBlockHash();
        break;
    }
    default: // Don't crash the client on invalid blockType, just return a bad hash
        LogPrintf("---: ComputePoWHash HIGH HASH ERROR - PowType: %s \n", GetPoWType());
        return HIGH_HASH;
    }

    return powHash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, nFlags=%08x, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        nFlags, vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
