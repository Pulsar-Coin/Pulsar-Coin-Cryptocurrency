// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2022 The Pulsar developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_GENESIS_H
#define BITCOIN_GENESIS_H

#include <primitives/block.h>
#include <stdint.h>

CBlock CreateGenesisBlock(uint32_t nTimeBlock, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount &genesisReward);
void MineGenesisBlock(const CBlock& genesis, uint256 powLimit);

#endif // BITCOIN_GENESIS_H
