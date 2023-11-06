// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include <uint256.h>
#include <limits>
#include <map>
#include <string>
#include <amount.h>

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    DEPLOYMENT_CSV, // Deployment of BIP68, BIP112, and BIP113.
    DEPLOYMENT_SEGWIT, // Deployment of BIP141, BIP143, and BIP147.
    // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
    MAX_VERSION_BITS_DEPLOYMENTS
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    /** Block height at which BIP16 becomes active */
    int BIP16Height;
    /** Block height and hash at which BIP34 becomes active */
    int BIP34Height;
    uint256 BIP34Hash;
    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;

    int64_t nPowTargetSpacingGR;
    int64_t nPowTargetSpacingCH;


    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    int64_t DifficultyAdjustmentInterval() const { return nPowTargetTimespan / nPowTargetSpacing; }
    int64_t nPowTargetTimespan;
    uint32_t powForkTime;
    uint32_t isValid;
    std::vector<uint256> powTypeLimits; // Limits for each pow type (with future-proofing space; can't pick up NUM_BLOCK_TYPES here)

    uint32_t halvingForkBlock;
    uint32_t halvingFixForkBlock;

    /** pulsar stuff */
    int64_t nStakeTargetSpacing;
    int64_t nTargetSpacingWorkMax;
    int64_t nTargetTimespan;
    int64_t nStakeMinConfirmations;
    int64_t nModifierInterval;
  //  int64_t nTotalPOWBlock;
  //  int64_t warmUpPOWBlock;
    int nCoinbaseMaturity;  // Coinbase transaction outputs can only be spent after this number of new blocks (network rule)
    int nDgwPastBlocks;  // number of blocks to average in Dark Gravity Wave
    int nStartMiningTime;
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
