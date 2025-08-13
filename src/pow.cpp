// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2022 The Pulsar developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <pulsar.h>
#include <uint256.h>

#include <bignum.h>
#include <chainparams.h>
#include <util.h>

#include <validation.h>

// pulsar: find last block index up to pindex
const CBlockIndex *GetLastBlockIndex(const CBlockIndex *pindex, bool fProofOfStake, const POW_TYPE powType) {
    while (pindex && pindex->pprev && ((pindex->IsProofOfStake() != fProofOfStake) || (pindex->GetBlockHeader().GetPoWType() != powType)))
        pindex = pindex->pprev;
    return pindex;
}

double GetBlockDifficulty(uint32_t nBits) {
    int nShift = (nBits >> 24) & 0xff;

    double dDiff =
            (double) 0x0000ffff / (double) (nBits & 0x00ffffff);

    while (nShift < 31) {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 31) {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}

unsigned int DarkGravityWave(const CBlockIndex *pindexLast, bool fProofOfStake, const Consensus::Params &params, const POW_TYPE powType) {
    /* current difficulty formula, veil - DarkGravity v3, written by Evan Duffield - evan@dash.org */
    const arith_uint256 bnProofOfWorkLimit = UintToArith256(params.powTypeLimits[powType]);
    unsigned int nProofOfWorkLimit = bnProofOfWorkLimit.GetCompact();

    int nCountBlocks = 0;
    const CBlockIndex *pindex = pindexLast;
    const CBlockIndex *pindexLastMatchingProof = nullptr;
    arith_uint256 bnPastTargetAvg = 0;


	int match = 0;
	int nomatch = 0;

    while (nCountBlocks < params.nDgwPastBlocks) {
        // Ran out of blocks, return pow limit
        if (!pindex)
            return nProofOfWorkLimit;

	
        // Only consider PoW or PoS blocks but not both
        if (pindex->IsProofOfStake() != fProofOfStake || pindex->GetBlockHeader().GetPoWType() != powType) 
	{
		nomatch++;
		//LogPrint(BCLog::ALL, "--- NO MATCH - PowType: %s PoS: %s\n", pindex->GetBlockHeader().GetPoWType(), pindex->IsProofOfStake());
            pindex = pindex->pprev;
            continue;

        } else if (!pindexLastMatchingProof) {
            pindexLastMatchingProof = pindex;
        }
		match++;
	//LogPrint(BCLog::ALL, "--- MATCH - PowType: %s PoS: %s\n", pindex->GetBlockHeader().GetPoWType(), pindex->IsProofOfStake());

        arith_uint256 bnTarget = arith_uint256().SetCompact(pindex->nBits);
        bnPastTargetAvg = (bnPastTargetAvg * nCountBlocks + bnTarget) / (nCountBlocks + 1);

        if (++nCountBlocks != params.nDgwPastBlocks)
            pindex = pindex->pprev;
    }
	LogPrint(BCLog::ALL, "PIndex - PowType: %s PoS: %s Match: %s NoMatch %s \n", powType, fProofOfStake, match, nomatch);
    LogPrint(BCLog::ALL, "DarkGravityWave fProofOfStake=%s, pindexDGWHeight=%d, pindexLastMatchingProof=%d, nDgwPastBlocks=%d \n", fProofOfStake, pindex->nHeight, pindexLastMatchingProof->nHeight, params.nDgwPastBlocks);

    arith_uint256 bnNew(bnPastTargetAvg);
    LogPrint(BCLog::ALL, "-- avg diff %d\n", GetBlockDifficulty(bnNew.GetCompact()));

    int64_t nActualTimespan = pindexLastMatchingProof->GetBlockTime() - pindex->GetBlockTime();
    int64_t nTargetTimespan = params.nDgwPastBlocks * params.nStakeTargetSpacing;

    if (IsReductionActive(pindex->pprev, params))
    {
        nTargetTimespan = params.nDgwPastBlocks * params.nPoSTargetSpacing_Reduction;
    }

	if (!fProofOfStake)
	{
		const arith_uint256 powLimit = UintToArith256(params.powTypeLimits[powType]);   // Minimum diff, 00000ffff... for x16rt, 000fffff... for minotaurx

		if (IsMinoEnabled(pindex->pprev, params))
		{
			if (IsReductionActive(pindex->pprev, params))
			{
                if (powType == POW_TYPE_CURVEHASH) {
                    nTargetTimespan = params.nDgwPastBlocks * params.nPowTargetSpacingCurvehash_Reduction;
                    nProofOfWorkLimit = powLimit.GetCompact();
                } 
                else {
                    nTargetTimespan = params.nDgwPastBlocks * params.nPowTargetSpacingMinotaurX_Reduction;
                    nProofOfWorkLimit = powLimit.GetCompact();
                }

			} 
            else {
                if (powType == POW_TYPE_CURVEHASH) {
                    nTargetTimespan = params.nDgwPastBlocks * params.nPowTargetSpacingCH;
                    nProofOfWorkLimit = powLimit.GetCompact();
                } 
                else {
                    nTargetTimespan = params.nDgwPastBlocks * params.nPowTargetSpacingMino;
                    nProofOfWorkLimit = powLimit.GetCompact();
                }
            }
		}
		else
		{
			LogPrint(BCLog::ALL, "-- MINOTAURX NOT ENABLED --");
		}
	}

    LogPrint(BCLog::ALL, "-- PowType=%s nActualTimespan=%d nTargetTimespan=%d nDgwPastBlocks=%d\n", powType, nActualTimespan, nTargetTimespan, params.nDgwPastBlocks);

    if (nActualTimespan < nTargetTimespan / 3)
        nActualTimespan = nTargetTimespan / 3;
    if (nActualTimespan > nTargetTimespan * 3)
        nActualTimespan = nTargetTimespan * 3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnProofOfWorkLimit) {
        bnNew = bnProofOfWorkLimit;
	LogPrint(BCLog::ALL, "-- Return PoWLimit PowType=%s\n", powType);
    }
    LogPrint(BCLog::ALL, "-- next diff %d\n", GetBlockDifficulty(bnNew.GetCompact()));
    return bnNew.GetCompact();
}

unsigned int GetNextTargetRequired(const CBlockIndex *pindexLast, bool fProofOfStake, const Consensus::Params &params, const POW_TYPE powType) {
    
    const arith_uint256 bnProofOfWorkLimit = UintToArith256(params.powTypeLimits[powType]);
    unsigned int nProofOfWorkLimit = bnProofOfWorkLimit.GetCompact();

    if (params.fPowNoRetargeting) // regtest only
        return nProofOfWorkLimit;
    if (pindexLast == nullptr)
        return nProofOfWorkLimit; // genesis block
    return DarkGravityWave(pindexLast, fProofOfStake, params, powType);
}


bool CheckProofOfWork(const CBlockHeader *pblock, const Consensus::Params &params, bool cache) {

    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;
    bnTarget.SetCompact(pblock->nBits, &fNegative, &fOverflow);
    uint256 PoWHash;

	POW_TYPE powType = pblock->GetPoWType();
	// Check range
	if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powTypeLimits[powType])) {
		LogPrint(BCLog::ALL, "print bnTarget > UintToArith256(params.powLimit) = %s\n", bnTarget > UintToArith256(params.powTypeLimits[powType]));
        LogPrintf("print bnTarget > UintToArith256(params.powLimit) = %s\n", bnTarget > UintToArith256(params.powTypeLimits[powType]));
		return false;
	}

    if (cache)
        PoWHash = pblock->GetBlockHash();
    else
        PoWHash = pblock->GetBlockHash(false);

	if (UintToArith256(PoWHash) > bnTarget) {
		LogPrint(BCLog::ALL, "Hash > Target: bnTarget = %s, PoWHash=%s powType=%s \n", bnTarget.ToString(), PoWHash.ToString(), powType);
        LogPrintf("Hash > Target: bnTarget = %s, PoWHash=%s powType=%s \n", bnTarget.ToString(), PoWHash.ToString(), powType);
		return false;
	}

	return true;
}
