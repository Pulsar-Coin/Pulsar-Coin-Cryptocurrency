// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2022 The Pulsar developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include <consensus/params.h>

#include "primitives/block.h"

#include <stdint.h>

class CBlockHeader;
class CBlockIndex;
class uint256;

const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake, const POW_TYPE powType=POW_TYPE_CURVEHASH);
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake, const Consensus::Params& params, const POW_TYPE powType);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(const CBlockHeader *pblock, const Consensus::Params&, bool cache=true);

#endif // BITCOIN_POW_H

