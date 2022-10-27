// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/consensus.h>
#include <primitives/block.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>
#include <crypto/minotaurx/minotaur.h>
#include <validation.h>
#include <util.h>


BlockNetwork bNetwork = BlockNetwork();

BlockNetwork::BlockNetwork()
{
    fOnTestnet = false;
    fOnRegtest = false;
}

void BlockNetwork::SetNetwork(const std::string& net)
{
    if (net == "test") {
        fOnTestnet = true;
    } else if (net == "regtest") {
        fOnRegtest = true;
    }
}

uint256 CBlockHeader::ComputePoWHash() const
{
	//LogPrint(BCLog::ALL, "---BLOCK.cpp  - PowType: %s\n", GetPoWType());
            switch (GetPoWType()) {
            case POW_TYPE_CURVEHASH: {
		return SerializeHash(*this);
                break;
            }
            case POW_TYPE_MINOTAURX: {
                return Minotaur(BEGIN(nVersion), END(nNonce), true);
                break;
            }
            default: // Don't crash the client on invalid blockType, just return a bad hash
		LogPrint(BCLog::ALL, "---ERROR: BLOCK.cpp  - PowType: %s\n", GetPoWType());
                return HIGH_HASH;
            }
}





uint256 CBlockHeader::GetHash() const
{
    uint256 powHash;

    powHash = ComputePoWHash();
	
    return powHash;
}
/*
uint256 CBlockHeader::CrowHashArbitrary(const char* data) {
    return Minotaur(data, data + strlen(data), true);
}
*/
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
