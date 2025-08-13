// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <miner.h>

#include <amount.h>
#include <chain.h>
#include <chainparams.h>
#include <coins.h>
#include <consensus/consensus.h>
#include <consensus/tx_verify.h>
#include <consensus/merkle.h>
#include <consensus/validation.h>
#include <validation.h>
#include <net.h>
#include <policy/policy.h>
#include <pow.h>
#include <primitives/transaction.h>
#include <script/standard.h>
#include <timedata.h>
#include <util.h>
#include <utilmoneystr.h>
#include <validationinterface.h>

#include <wallet/wallet.h>
#include <warnings.h>
#include <base58.h>

#include <algorithm>
#include <memory>
#include <queue>
#include <utility>

#include <boost/thread.hpp>

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockWeight = 0;

std::atomic<bool> fStopMinerProc(false);
std::atomic<bool> fTryToSync(false);
std::atomic<bool> fIsStaking(false);

int64_t UpdateTime(CBlockHeader *pblock, const Consensus::Params& Params, const CBlockIndex* pindexPrev, const POW_TYPE powType) {

    int64_t nOldTime = pblock->nTime;
    int64_t nNewTime = std::max(pblock->GetBlockTime(), GetAdjustedTime());

    if (nOldTime < nNewTime)
        pblock->nTime = nNewTime;

    if (IsMinoEnabled(pindexPrev, Params)) {
         if (Params.fPowAllowMinDifficultyBlocks)
            pblock->nBits = GetNextTargetRequired(pindexPrev, pblock, Params, powType);
    } else {
        if (Params.fPowAllowMinDifficultyBlocks)
            pblock->nBits = GetNextTargetRequired(pindexPrev, pblock, Params, powType);
    }

    return nNewTime - nOldTime;
}

BlockAssembler::Options::Options() {
    nBlockMaxWeight = DEFAULT_BLOCK_MAX_WEIGHT;
}

BlockAssembler::BlockAssembler(const CChainParams &params, const Options &options) : chainparams(params) {
    // Limit weight to between 4K and MAX_BLOCK_WEIGHT-4K for sanity:
    nBlockMaxWeight = std::max<size_t>(4000, std::min<size_t>(MAX_BLOCK_WEIGHT - 4000, options.nBlockMaxWeight));
}

static BlockAssembler::Options DefaultOptions(const CChainParams &params) {
    // Block resource limits
    // If -blockmaxweight is not given, limit to DEFAULT_BLOCK_MAX_WEIGHT
    BlockAssembler::Options options;
    options.nBlockMaxWeight = gArgs.GetArg("-blockmaxweight", DEFAULT_BLOCK_MAX_WEIGHT);
    return options;
}

BlockAssembler::BlockAssembler(const CChainParams &params) : BlockAssembler(params, DefaultOptions(params)) {}

void BlockAssembler::resetBlock() {
    inBlock.clear();

    // Reserve space for coinbase tx
    nBlockWeight = 4000;
    nBlockSigOpsCost = 400;
    fIncludeWitness = false;

    // These counters do not include coinbase tx
    nBlockTx = 0;
    nFees = 0;
}

// pulsar: if pwallet != NULL it will attempt to create coinstake
std::unique_ptr <CBlockTemplate> BlockAssembler::CreateNewBlock(const CScript &scriptPubKeyIn, bool fMineWitnessTx, CWallet *pwallet, bool *pfPoSCancel, const POW_TYPE powType) {
    int64_t nTimeStart = GetTimeMicros();

    resetBlock();

    pblocktemplate.reset(new CBlockTemplate());

    if (!pblocktemplate.get())
        return nullptr;
    pblock = &pblocktemplate->block; // pointer for convenience

    LOCK(cs_main);
    CBlockIndex *pindexPrev = chainActive.Tip();
    assert(pindexPrev != nullptr);
    nHeight = pindexPrev->nHeight + 1;
    unsigned int nPOWBlockHeight = pindexPrev->nPOWBlockHeight + 1;

    // Refuse to attempt to create a non-curvehsh block before activation
    if (!IsMinoEnabled(pindexPrev, chainparams.GetConsensus()) && powType != 0)
        throw std::runtime_error("Error: Won't attempt to create a non-curvehash block before minotaurx activation");

    // If GR Algo is enabled, encode desired pow type.
    if (IsMinoEnabled(pindexPrev, chainparams.GetConsensus())) {
        if (powType >= NUM_BLOCK_TYPES)
            throw std::runtime_error("Error: Unrecognised pow type requested");
        pblock->nVersion |= powType << 16;
    }

    // Create coinbase transaction.
    CMutableTransaction coinbaseTx;
    coinbaseTx.vin.resize(1);
    coinbaseTx.vin[0].prevout.SetNull();
    coinbaseTx.vout.resize(1);
    coinbaseTx.vout[0].scriptPubKey = scriptPubKeyIn;
    CAmount blockValue = 0;
    if (pblock->IsProofOfWork()) {
        pblock->nBits = GetNextTargetRequired(pindexPrev, false, chainparams.GetConsensus(), powType);
	if (IsHalvingActive(pindexPrev, Params().GetConsensus()))
	{
		blockValue = GetBlockReward(nHeight);
	}
	else
	{
        	blockValue = GetProofOfWorkReward(nPOWBlockHeight);
	}
        coinbaseTx.vout[0].nValue = blockValue;
    }

    // Add dummy coinbase tx as first transaction
    pblock->vtx.emplace_back();
    pblocktemplate->vTxFees.push_back(-1); // updated at end
    pblocktemplate->vTxSigOpsCost.push_back(-1); // updated at end

    // pulsar: if coinstake available add coinstake tx
    static int64_t nLastCoinStakeSearchTime = GetAdjustedTime();  // only initialized at startup

    if (pwallet)  // attemp to find a coinstake
    {
        *pfPoSCancel = true;
        pblock->nBits = GetNextTargetRequired(pindexPrev, true, chainparams.GetConsensus(), powType);
        CMutableTransaction txCoinStake;
        int64_t nSearchTime = txCoinStake.nTime; // search to current time
        if (nSearchTime > nLastCoinStakeSearchTime)
        {
		int64_t nStart = GetTimeMicros();
            if (pwallet->CreateCoinStake(*pwallet, pblock->nBits, txCoinStake))
            {
                if (txCoinStake.nTime >= std::max(pindexPrev->GetMedianTimePast()+1, pindexPrev->GetBlockTime() - MAX_FUTURE_BLOCK_TIME))
                {   // make sure coinstake would meet timestamp protocol
                    // as it would be the same as the block timestamp
                    coinbaseTx.vout[0].SetEmpty();
                    coinbaseTx.nTime = txCoinStake.nTime;
                    pblock->vtx.push_back(MakeTransactionRef(CTransaction(txCoinStake)));
                    *pfPoSCancel = false;
                }
            }
            nLastCoinStakeSearchTime = nSearchTime;
		int64_t nFinish = GetTimeMicros();
		int64_t nDiff = 0.001 * (nFinish - nStart);
		if (nDiff > 1000)
		{
    			LogPrintf("Input Timer: %.0fms\n", static_cast<double>(nDiff));
		}
		else
		{
			LogPrint(BCLog::ALL, "Input Timer: %.0fms\n", static_cast<double>(nDiff));
		}
        }
        if (*pfPoSCancel)
            return nullptr; // pulsar: there is no point to continue if we failed to create coinstake
    }

    LOCK(mempool.cs);

    // -regtest only: allow overriding block.nVersion with
    // -blockversion=N to test forking scenarios
    if (chainparams.MineBlocksOnDemand())
        pblock->nVersion = gArgs.GetArg("-blockversion", pblock->nVersion);

    pblock->nTime = GetAdjustedTime();
    const int64_t nMedianTimePast = pindexPrev->GetMedianTimePast();

    nLockTimeCutoff = (STANDARD_LOCKTIME_VERIFY_FLAGS & LOCKTIME_MEDIAN_TIME_PAST)
                      ? nMedianTimePast
                      : pblock->GetBlockTime();

    // Decide whether to include witness transactions
    // This is only needed in case the witness softfork activation is reverted
    // (which would require a very deep reorganization) or when
    // -promiscuousmempoolflags is used.
    // TODO: replace this with a call to main to assess validity of a mempool
    // transaction (which in most cases can be a no-op).
    fIncludeWitness = IsWitnessEnabled(pindexPrev, chainparams.GetConsensus()) && fMineWitnessTx;

    int nPackagesSelected = 0;
    int nDescendantsUpdated = 0;
    addPackageTxs(nPackagesSelected, nDescendantsUpdated);

    int64_t nTime1 = GetTimeMicros();

    nLastBlockTx = nBlockTx;
    nLastBlockWeight = nBlockWeight;

    coinbaseTx.vin[0].scriptSig = CScript() << nHeight << OP_0;
    pblock->vtx[0] = MakeTransactionRef(std::move(coinbaseTx));
    if (fIncludeWitness)
        pblocktemplate->vchCoinbaseCommitment = GenerateCoinbaseCommitment(*pblock, pindexPrev, chainparams.GetConsensus());
    pblocktemplate->vTxFees[0] = -nFees;

    LogPrintf("CreateNewBlock(): block weight: %u txs: %u fees: %ld sigops %d\n", GetBlockWeight(*pblock), nBlockTx, nFees, nBlockSigOpsCost);

    // Fill in header
    pblock->hashPrevBlock = pindexPrev->GetBlockHash();
    if (pblock->IsProofOfStake())
        pblock->nTime = pblock->vtx[1]->nTime; //same as coinstake timestamp

    pblock->nTime = std::max(pindexPrev->GetMedianTimePast() + 1, pblock->GetMaxTransactionTime());
    pblock->nTime = std::max(pblock->GetBlockTime(), pindexPrev->GetBlockTime() - MAX_FUTURE_BLOCK_TIME);

    if (pblock->IsProofOfWork())
    {
        UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev, powType);
	    
	if (IsMinoEnabled(pindexPrev, chainparams.GetConsensus())) {
		//pblock->nBits = GetNextTargetRequired(pindexPrev, pblock, chainparams.GetConsensus(), powType);
		pblock->nBits = GetNextTargetRequired(pindexPrev, false, chainparams.GetConsensus(), powType);
	} else {

		//pblock->nBits = GetNextTargetRequired(pindexPrev, pblock, chainparams.GetConsensus(), powType);
		pblock->nBits = GetNextTargetRequired(pindexPrev, false, chainparams.GetConsensus(), powType);
	}
    }

    pblock->nNonce = 0;
    pblocktemplate->vTxSigOpsCost[0] = WITNESS_SCALE_FACTOR * GetLegacySigOpCount(*pblock->vtx[0]);

    CValidationState state;
    if (pwallet && !TestBlockValidity(state, chainparams, *pblock, pindexPrev, false, false)) {
        throw std::runtime_error(strprintf("%s: TestBlockValidity failed: %s", __func__, FormatStateMessage(state)));
    }

    int64_t nTime2 = GetTimeMicros();

    LogPrint(BCLog::BENCH, "CreateNewBlock() packages: %.2fms (%d packages, %d updated descendants), validity: %.2fms (total %.2fms)\n", 0.001 * (nTime1 - nTimeStart), nPackagesSelected,
             nDescendantsUpdated, 0.001 * (nTime2 - nTime1), 0.001 * (nTime2 - nTimeStart));

    return std::move(pblocktemplate);
}

void BlockAssembler::onlyUnconfirmed(CTxMemPool::setEntries &testSet) {
    for (CTxMemPool::setEntries::iterator iit = testSet.begin(); iit != testSet.end();) {
        // Only test txs not already in the block
        if (inBlock.count(*iit)) {
            testSet.erase(iit++);
        } else {
            iit++;
        }
    }
}

bool BlockAssembler::TestPackage(uint64_t packageSize, int64_t packageSigOpsCost) const {
    // TODO: switch to weight-based accounting for packages instead of vsize-based accounting.
    if (nBlockWeight + WITNESS_SCALE_FACTOR * packageSize >= nBlockMaxWeight)
        return false;
    if (nBlockSigOpsCost + packageSigOpsCost >= MAX_BLOCK_SIGOPS_COST)
        return false;
    return true;
}

// Perform transaction-level checks before adding to block:
// - transaction finality (locktime)
// - premature witness (in case segwit transactions are added to mempool before
//   segwit activation)
bool BlockAssembler::TestPackageTransactions(const CTxMemPool::setEntries &package) {
    for (const CTxMemPool::txiter it : package) {
        if (!IsFinalTx(it->GetTx(), nHeight, nLockTimeCutoff))
            return false;
        if (!fIncludeWitness && it->GetTx().HasWitness())
            return false;

        // pulsar: timestamp limit
        if (it->GetTx().nTime > GetAdjustedTime() || (pblock->IsProofOfStake() && it->GetTx().nTime > pblock->vtx[1]->nTime))
            return false;
    }
    return true;
}

void BlockAssembler::AddToBlock(CTxMemPool::txiter iter) {
    pblock->vtx.emplace_back(iter->GetSharedTx());
    pblocktemplate->vTxFees.push_back(iter->GetFee());
    pblocktemplate->vTxSigOpsCost.push_back(iter->GetSigOpCost());
    nBlockWeight += iter->GetTxWeight();
    ++nBlockTx;
    nBlockSigOpsCost += iter->GetSigOpCost();
    nFees += iter->GetFee();
    inBlock.insert(iter);

    bool fPrintPriority = gArgs.GetBoolArg("-printpriority", DEFAULT_PRINTPRIORITY);
    if (fPrintPriority) {
        LogPrintf("fee %d satoshi txid %s\n",
                  iter->GetModifiedFee(),
                  iter->GetTx().GetHash().ToString());
    }
}

int BlockAssembler::UpdatePackagesForAdded(const CTxMemPool::setEntries &alreadyAdded,
                                           indexed_modified_transaction_set &mapModifiedTx) {
    int nDescendantsUpdated = 0;
    for (const CTxMemPool::txiter it : alreadyAdded) {
        CTxMemPool::setEntries descendants;
        mempool.CalculateDescendants(it, descendants);
        // Insert all descendants (not yet in block) into the modified set
        for (CTxMemPool::txiter desc : descendants) {
            if (alreadyAdded.count(desc))
                continue;
            ++nDescendantsUpdated;
            modtxiter mit = mapModifiedTx.find(desc);
            if (mit == mapModifiedTx.end()) {
                CTxMemPoolModifiedEntry modEntry(desc);
                modEntry.nSizeWithAncestors -= it->GetTxSize();
                modEntry.nModFeesWithAncestors -= it->GetModifiedFee();
                modEntry.nSigOpCostWithAncestors -= it->GetSigOpCost();
                mapModifiedTx.insert(modEntry);
            } else {
                mapModifiedTx.modify(mit, update_for_parent_inclusion(it));
            }
        }
    }
    return nDescendantsUpdated;
}

// Skip entries in mapTx that are already in a block or are present
// in mapModifiedTx (which implies that the mapTx ancestor state is
// stale due to ancestor inclusion in the block)
// Also skip transactions that we've already failed to add. This can happen if
// we consider a transaction in mapModifiedTx and it fails: we can then
// potentially consider it again while walking mapTx.  It's currently
// guaranteed to fail again, but as a belt-and-suspenders check we put it in
// failedTx and avoid re-evaluation, since the re-evaluation would be using
// cached size/sigops/fee values that are not actually correct.
bool BlockAssembler::SkipMapTxEntry(CTxMemPool::txiter it, indexed_modified_transaction_set &mapModifiedTx, CTxMemPool::setEntries &failedTx) {
    assert(it != mempool.mapTx.end());
    return mapModifiedTx.count(it) || inBlock.count(it) || failedTx.count(it);
}

void BlockAssembler::SortForBlock(const CTxMemPool::setEntries &package, CTxMemPool::txiter entry, std::vector <CTxMemPool::txiter> &sortedEntries) {
    // Sort package by ancestor count
    // If a transaction A depends on transaction B, then A's ancestor count
    // must be greater than B's.  So this is sufficient to validly order the
    // transactions for block inclusion.
    sortedEntries.clear();
    sortedEntries.insert(sortedEntries.begin(), package.begin(), package.end());
    std::sort(sortedEntries.begin(), sortedEntries.end(), CompareTxIterByAncestorCount());
}

// This transaction selection algorithm orders the mempool based
// on feerate of a transaction including all unconfirmed ancestors.
// Since we don't remove transactions from the mempool as we select them
// for block inclusion, we need an alternate method of updating the feerate
// of a transaction with its not-yet-selected ancestors as we go.
// This is accomplished by walking the in-mempool descendants of selected
// transactions and storing a temporary modified state in mapModifiedTxs.
// Each time through the loop, we compare the best transaction in
// mapModifiedTxs with the next transaction in the mempool to decide what
// transaction package to work on next.
void BlockAssembler::addPackageTxs(int &nPackagesSelected, int &nDescendantsUpdated) {
    // mapModifiedTx will store sorted packages after they are modified
    // because some of their txs are already in the block
    indexed_modified_transaction_set mapModifiedTx;
    // Keep track of entries that failed inclusion, to avoid duplicate work
    CTxMemPool::setEntries failedTx;

    // Start by adding all descendants of previously added txs to mapModifiedTx
    // and modifying them for their already included ancestors
    UpdatePackagesForAdded(inBlock, mapModifiedTx);

    CTxMemPool::indexed_transaction_set::index<ancestor_score>::type::iterator mi = mempool.mapTx.get<ancestor_score>().begin();
    CTxMemPool::txiter iter;

    // Limit the number of attempts to add transactions to the block when it is
    // close to full; this is just a simple heuristic to finish quickly if the
    // mempool has a lot of entries.
    const int64_t MAX_CONSECUTIVE_FAILURES = 1000;
    int64_t nConsecutiveFailed = 0;

    while (mi != mempool.mapTx.get<ancestor_score>().end() || !mapModifiedTx.empty()) {
        // First try to find a new transaction in mapTx to evaluate.
        if (mi != mempool.mapTx.get<ancestor_score>().end() &&
            SkipMapTxEntry(mempool.mapTx.project<0>(mi), mapModifiedTx, failedTx)) {
            ++mi;
            continue;
        }

        // Now that mi is not stale, determine which transaction to evaluate:
        // the next entry from mapTx, or the best from mapModifiedTx?
        bool fUsingModified = false;

        modtxscoreiter modit = mapModifiedTx.get<ancestor_score>().begin();
        if (mi == mempool.mapTx.get<ancestor_score>().end()) {
            // We're out of entries in mapTx; use the entry from mapModifiedTx
            iter = modit->iter;
            fUsingModified = true;
        } else {
            // Try to compare the mapTx entry to the mapModifiedTx entry
            iter = mempool.mapTx.project<0>(mi);
            if (modit != mapModifiedTx.get<ancestor_score>().end() &&
                CompareTxMemPoolEntryByAncestorFee()(*modit, CTxMemPoolModifiedEntry(iter))) {
                // The best entry in mapModifiedTx has higher score
                // than the one from mapTx.
                // Switch which transaction (package) to consider
                iter = modit->iter;
                fUsingModified = true;
            } else {
                // Either no entry in mapModifiedTx, or it's worse than mapTx.
                // Increment mi for the next loop iteration.
                ++mi;
            }
        }

        // We skip mapTx entries that are inBlock, and mapModifiedTx shouldn't
        // contain anything that is inBlock.
        assert(!inBlock.count(iter));

        uint64_t packageSize = iter->GetSizeWithAncestors();
        int64_t packageSigOpsCost = iter->GetSigOpCostWithAncestors();
        if (fUsingModified) {
            packageSize = modit->nSizeWithAncestors;
            packageSigOpsCost = modit->nSigOpCostWithAncestors;
        }

        if (!TestPackage(packageSize, packageSigOpsCost)) {
            if (fUsingModified) {
                // Since we always look at the best entry in mapModifiedTx,
                // we must erase failed entries so that we can consider the
                // next best entry on the next loop iteration
                mapModifiedTx.get<ancestor_score>().erase(modit);
                failedTx.insert(iter);
            }

            ++nConsecutiveFailed;

            if (nConsecutiveFailed > MAX_CONSECUTIVE_FAILURES && nBlockWeight >
                                                                 nBlockMaxWeight - 4000) {
                // Give up if we're close to full and haven't succeeded in a while
                break;
            }
            continue;
        }

        CTxMemPool::setEntries ancestors;
        uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
        std::string dummy;
        mempool.CalculateMemPoolAncestors(*iter, ancestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, false);

        onlyUnconfirmed(ancestors);
        ancestors.insert(iter);

        // Test if all tx's are Final
        if (!TestPackageTransactions(ancestors)) {
            if (fUsingModified) {
                mapModifiedTx.get<ancestor_score>().erase(modit);
                failedTx.insert(iter);
            }
            continue;
        }

        // This transaction will make it in; reset the failed counter.
        nConsecutiveFailed = 0;

        // Package can be added. Sort the entries in a valid order.
        std::vector <CTxMemPool::txiter> sortedEntries;
        SortForBlock(ancestors, iter, sortedEntries);

        for (size_t i = 0; i < sortedEntries.size(); ++i) {
            AddToBlock(sortedEntries[i]);
            // Erase from the modified set, if present
            mapModifiedTx.erase(sortedEntries[i]);
        }

        ++nPackagesSelected;

        // Update transactions that depend on each of these
        nDescendantsUpdated += UpdatePackagesForAdded(ancestors, mapModifiedTx);
    }
}

void IncrementExtraNonce(CBlock *pblock, const CBlockIndex *pindexPrev, unsigned int &nExtraNonce) {
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock) {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(*pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
    pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
}


static bool ProcessBlockFound(const CBlock *pblock, const CChainParams &chainparams) {
    LogPrintf("%s\n", pblock->ToString());
    LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0]->vout[0].nValue));

    // Found a solution
    {
//        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("PulsarMiner: generated block is stale");
    }

    // Process this block the same as if we had received it from another node
    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
    if (!ProcessNewBlock(Params(), shared_pblock, true, NULL))
        return error("ProcessNewBlock, block not accepted");

    return true;
}

void PoSMiner(CWallet *pwallet) {
    LogPrintf("PoSMiner started for proof-of-stake\n");
    RenameThread("pulsar-stake-minter");

    unsigned int nExtraNonce = 0;

    std::shared_ptr <CReserveScript> coinbaseScript;
    pwallet->GetScriptForMining(coinbaseScript);

    // Compute timeout for pos as sqrt(numUTXO)
    unsigned int pos_timio;
    {
        std::vector <COutput> vCoins;
        pwallet->AvailableCoins(vCoins, false);
        pos_timio = gArgs.GetArg("-staketimio", 500) + 6 * sqrt(vCoins.size());
        LogPrintf("Set proof-of-stake timeout: %ums for %u UTXOs\n", pos_timio, vCoins.size());
    }

    std::string strMintMessage = _("Info: Staking suspended due to locked wallet.");
    std::string strMintSyncMessage = _("Info: Staking suspended while synchronizing wallet.");
    std::string strMintDisabledMessage = _("Info: Staking disabled by set 0 to 'staking' option.");
    std::string strMintBlockMessage = _("Info: Staking suspended due to block creation failure.");
    std::string strMintEmpty = _("");
    if (!gArgs.GetBoolArg("-staking", true)) {
        strMintWarning = strMintDisabledMessage;
        LogPrintf("proof-of-stake minter disabled\n");
        return;
    }

    try {

        // Throw an error if no script was provided.  This can happen
        // due to some internal error but also if the keypool is empty.
        // In the latter case, already the pointer is NULL.
        if (!coinbaseScript || coinbaseScript->reserveScript.empty())
            throw std::runtime_error("No coinbase script available (mining requires a wallet)");

        while (true) {
            while (pwallet->IsLocked()) {
                LogPrintf("wallet is locked\n");
                strMintWarning = strMintMessage;
                MilliSleep(5000);
            }
            unsigned int nMiningRequiresPeers = Params().MiningRequiresPeers();
            if (nMiningRequiresPeers > 0) {
                // Busy-wait for the network to come online so we don't waste time mining
                // on an obsolete chain. In regtest mode we expect to fly solo.
                while (g_connman == nullptr || g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) < nMiningRequiresPeers || IsInitialBlockDownload())
                    MilliSleep(5 * 1000);
            }
            while (GuessVerificationProgress(Params().TxData(), chainActive.Tip()) < 0.996) {
                LogPrintf("Minter thread sleeps while sync at %f\n", GuessVerificationProgress(Params().TxData(), chainActive.Tip()));
                strMintWarning = strMintSyncMessage;
                MilliSleep(10000);
            }

            strMintWarning = strMintEmpty;

	    //ADDED

            //
            // Create new block
            //
            CBlockIndex *pindexPrev = chainActive.Tip();
            bool fPoSCancel = false;
            std::unique_ptr <CBlockTemplate> pblocktemplate(BlockAssembler(Params()).CreateNewBlock(coinbaseScript->reserveScript, true, pwallet, &fPoSCancel, POW_TYPE_CURVEHASH));
            if (!pblocktemplate.get()) {
                if (fPoSCancel == true) {
                    MilliSleep(pos_timio);
                    continue;
                }
                strMintWarning = strMintBlockMessage;
                LogPrintf("Error in PoSMiner: Keypool ran out, please call keypoolrefill before restarting the mining thread\n");
                return;
            }
            CBlock *pblock = &pblocktemplate->block;
            IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

            // pulsar: if proof-of-stake block found then process block
            if (pblock->IsProofOfStake()) {
                pblock->nFlags = CBlockIndex::BLOCK_PROOF_OF_STAKE;
                if (!SignBlock(*pblock, *pwallet)) {
                    LogPrintf("PoSMiner(): failed to sign PoS block");
                    continue;
                }
                LogPrintf("PoSMiner : proof-of-stake block found %s\n", pblock->GetHash().ToString());
                ProcessBlockFound(pblock, Params());
                // Rest for ~2 minutes after successful block to preserve close quick
                MilliSleep(60 * 1000 + GetRand(1 * 60 * 1000));
            }
            MilliSleep(pos_timio);

            continue;
        }
    }
    catch (boost::thread_interrupted) {
        LogPrintf("PoSMiner terminated\n");
        return;
        // throw;
    }
    catch (const std::runtime_error &e) {
        LogPrintf("PoSMiner runtime error: %s\n", e.what());
        return;
    }
}

// pulsar: stake minter thread
void static ThreadStakeMinter(void *parg) {
    LogPrintf("ThreadStakeMinter started\n");
    CWallet *pwallet = (CWallet *) parg;
    try {
        PoSMiner(pwallet);
    }
    catch (std::exception &e) {
        PrintExceptionContinue(&e, "ThreadStakeMinter()");
    } catch (...) {
        PrintExceptionContinue(NULL, "ThreadStakeMinter()");
    }
    LogPrintf("ThreadStakeMinter exiting\n");
}

// pulsar: stake minter
void MintStake(boost::thread_group &threadGroup) {
    // pulsar: mint proof-of-stake blocks in the background
    if (!vpwallets.empty())
        threadGroup.create_thread(boost::bind(&ThreadStakeMinter, vpwallets[0]));
}

//////////////////////////////////////////////////////////////////////////////
//
// Internal miner
//

//
// ScanHash scans nonces looking for a hash with at least some zero bits.
// The nonce is usually preserved between calls, but periodically or if the
// nonce is 0xffff0000 or above, the block is rebuilt and nNonce starts over at
// zero.
//
bool static ScanHash(const CBlockHeader *pblock, uint32_t &nNonce, uint256 *phash) {
    while (true) {
        nNonce++;

        // Calculate Pulsar for given block and nonce
        Pulsar(pblock, nNonce, phash);

        // Return the nonce if the hash has at least some zero bits,
        // caller will check if it has enough to reach the target
        if (((uint16_t *) phash)[15] == 0)
            return true;

        // If nothing found after trying for a while, return -1
        if ((nNonce & 0xfff) == 0) {
            return false;
        }
    }
}

void static PulsarMiner(const CChainParams &chainparams, void *parg, const POW_TYPE powType) {
    LogPrintf("PulsarMiner started\n");
    RenameThread("pulsar-miner");
    CWallet *pwallet = (CWallet *) parg;

    unsigned int nExtraNonce = 0;

    std::shared_ptr <CReserveScript> coinbaseScript;
    pwallet->GetScriptForMining(coinbaseScript);

    try {
        // Throw an error if no script was provided.  This can happen
        // due to some internal error but also if the keypool is empty.
        // In the latter case, already the pointer is NULL.
        if (!coinbaseScript || coinbaseScript->reserveScript.empty())
            throw std::runtime_error("No coinbase script available (mining requires a wallet)");
        unsigned int nMiningRequiresPeers = Params().MiningRequiresPeers();
        uint32_t nNonce;
        uint256 hash;
        arith_uint256 hashTarget;
        int64_t nStart;
        unsigned int nTransactionsUpdatedLast;
        while (true) {
            if (nMiningRequiresPeers > 0) {
                // Busy-wait for the network to come online so we don't waste time mining
                // on an obsolete chain. In regtest mode we expect to fly solo.
                while (g_connman == nullptr || g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) < nMiningRequiresPeers)
                    MilliSleep(1000);
            }
            if (GetTime() < Params().GetConsensus().nStartMiningTime) {
                LogPrintf("It's not time to start minining, please wait more!\n");
                while (GetTime() < Params().GetConsensus().nStartMiningTime) {
                    MilliSleep(1000);
                }
            }

            MilliSleep(2000);
            //
            // Create new block
            //
            nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
            CBlockIndex *pindexPrev = chainActive.Tip();

            std::unique_ptr <CBlockTemplate> pblocktemplate(BlockAssembler(Params()).CreateNewBlock(coinbaseScript->reserveScript, true, nullptr, nullptr, powType));
            if (!pblocktemplate.get()) {
                LogPrintf("Error in PulsarMiner: Keypool ran out, please call keypoolrefill before restarting the mining hread\n");
                return;
            }
            CBlock *pblock = &pblocktemplate->block;
            IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

            LogPrintf("Running PulsarMiner with %u transactions in block (%u bytes)\n", pblock->vtx.size(),
                      ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

            //
            // Search
            //
            nStart = GetTime();
            arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);
            nNonce = 0;
            while (true) {
                // Check if something found
                if (ScanHash(pblock, nNonce, &hash)) {
                    if (UintToArith256(hash) <= hashTarget) {
                        // Found a solution
                        pblock->nNonce = nNonce;

                        GetCurvehashHash(pblock, &hash);

//                        LOCK2(cs_main, pwallet->cs_wallet);
                        if (!SignBlock(*pblock, *pwallet)) {
                            LogPrintf("PulsarMiner(): failed to sign PoS block");
                            continue;
                        }

                        LogPrintf("proof-of-work found  \n  hash: %s  \ntarget: %s\n", hash.GetHex(), hashTarget.GetHex());
                        ProcessBlockFound(pblock, chainparams);
                        LogPrintf("Done:\n");
                        coinbaseScript->KeepScript();

                        // In regression test mode, stop mining after a block is found.
                        if (chainparams.MineBlocksOnDemand())
                            throw boost::thread_interrupted();

                        break;
                    }
                }

                // Check for stop or if block needs to be rebuilt
                boost::this_thread::interruption_point();
                // Regtest mode doesn't require peers
                if ((g_connman == nullptr || g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) == 0) && chainparams.MiningRequiresPeers())
                    break;
                if (nNonce >= 0xffff0000)
                    break;
                if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                    break;
                if (pindexPrev != chainActive.Tip())
                    break;
                // Update nTime every few seconds
                //if (UpdateTime(pblock, powType) < 0)
		if(UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev, powType) < 0)
                    break; // Recreate the block if the clock has run backwards,
                // so that we can use the correct time.
                if (chainparams.GetConsensus().fPowAllowMinDifficultyBlocks) {
                    // Changing pblock->nTime can change work required on testnet:
                    hashTarget.SetCompact(pblock->nBits);
                }
            }
        }
        throw boost::thread_interrupted();
    }
    catch (const boost::thread_interrupted &e) {
        LogPrintf("PulsarMiner terminated!\n");
        throw;
    }
    catch (const std::runtime_error &e) {
        LogPrintf("PulsarMiner runtime error: %s\n", e.what());
        return;
    }
}

void GeneratePulsar(bool fGenerate, int nThreads, const CChainParams &chainparams) {
    static boost::thread_group *minerThreads = NULL;

    if (nThreads < 0)
        nThreads = GetNumCores();

    if (minerThreads != NULL) {
        minerThreads->interrupt_all();
        delete minerThreads;
        minerThreads = NULL;
    }

    if (nThreads == 0 || !fGenerate)
        return;

    minerThreads = new boost::thread_group();


    std::string strAlgo = gArgs.GetArg("-powalgo", DEFAULT_POW_TYPE);

    bool algoFound = false;
    POW_TYPE powType;
    for (unsigned int i = 0; i < NUM_BLOCK_TYPES; i++) {
        if (strAlgo == POW_TYPE_NAMES[i]) {
            powType = (POW_TYPE)i;
            algoFound = true;
            break;
        }
    }
    if (!algoFound)
        LogPrintf("PulsarMiner -- Invalid pow algorithm requested");



    if (!vpwallets.empty()) {
        for (int i = 0; i < nThreads; i++)
            minerThreads->create_thread(boost::bind(&PulsarMiner, boost::cref(chainparams), vpwallets[0], powType));
    }
}
