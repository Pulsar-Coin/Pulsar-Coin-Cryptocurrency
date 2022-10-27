// Copyright (c) 2017-2019 The Peercoin developers
// Copyright (c) 2022 The Pulsar developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PULSAR_KERNEL_H
#define PULSAR_KERNEL_H

#include <primitives/transaction.h> // CTransaction(Ref)

class CBlockIndex;
class CValidationState;
class CBlockHeader;
class CBlock;

// MODIFIER_INTERVAL_RATIO:
// ratio of group interval length between the last group and the first group
static const int MODIFIER_INTERVAL_RATIO = 3;

// Compute the hash modifier for proof-of-stake
uint256 ComputeStakeModifier(const CBlockIndex* pindexPrev, const uint256& kernel);

// Check whether stake kernel meets hash target
// Sets hashProofOfStake on success return
bool CheckStakeKernelHash(unsigned int nBits, CBlockIndex* pindexPrev, const CBlockHeader& blockFrom, const CTransactionRef& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, uint256& targetProofOfStake, bool fPrintProofOfStake=false);

// Check kernel hash target and coinstake signature
// Sets hashProofOfStake on success return
bool CheckProofOfStake(CValidationState &state, CBlockIndex* pindexPrev, const CTransactionRef &tx, unsigned int nBits, uint256& hashProofOfStake, uint256& targetProofOfStake);

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx);

/**
 * Wrapper around CheckStakeKernelHash()
 * Also checks existence of kernel input and min age
 * Convenient for searching a kernel
 */
bool CheckKernel(unsigned int nBits, CBlockIndex *pindexPrev, const CBlockHeader& blockFrom, const CTransactionRef& txPrev, const COutPoint& prevout, unsigned int nTime);

#endif // PULSAR_KERNEL_H
