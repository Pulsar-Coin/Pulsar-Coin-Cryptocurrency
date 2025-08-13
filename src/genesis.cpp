// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2022 The Pulsar developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <genesis.h>
#include <arith_uint256.h>
#include <chain.h>
#include <pulsar.h>
#include <consensus/merkle.h>
#include <primitives/block.h>
#include <uint256.h>
#include <utilstrencodings.h>

#include <bignum.h>
#include <util.h>

bool static ScanHash(const CBlockHeader *pblock, uint32_t &nNonce, uint256 *phash, uint32_t endNonce) {
    while (true) {
        nNonce++;
        if (nNonce % 10000 == 0) {
            printf("nTime %08u: nBits 0x%08x nonce %08u\n", pblock->nTime, pblock->nBits, nNonce);
        }

        // Calculate Pulsar for given block and nonce
        Pulsar(pblock, nNonce, phash);

        // Return the nonce if the hash has at least some zero bits,
        // caller will check if it has enough to reach the target
        if (((uint16_t *) phash)[15] == 0)
            return true;

        // If nothing found after trying for a while, return -1
        if (nNonce > endNonce)
            return false;
    }
}

CBlock CreateGenesisBlock(const char *pszTimestamp, const CScript &genesisOutputScript, uint32_t nTimeTx, uint32_t nTimeBlock, uint32_t nNonce, uint32_t nBits, int32_t nVersion,
                                 const CAmount &genesisReward) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig =
            CScript() << 486604799 << CScriptNum(9999) << std::vector<unsigned char>((const unsigned char *) pszTimestamp, (const unsigned char *) pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = 1369000 * COIN;
    txNew.vout[0].scriptPubKey = genesisOutputScript;
    txNew.nTime = nTimeTx;

    CBlock genesis;
    genesis.nTime = nTimeBlock;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
CBlock CreateGenesisBlock(uint32_t nTimeBlock, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount &genesisReward) {
    const char *pszTimestamp = "The Coronavirus Outbreak";
    const CScript genesisOutputScript = CScript() << ParseHex("04a60e7c8cd218fec9bf9565b04bd0d973608ec257c13d74ca7ee54243f18328175ef4be91f30fa747d4e358771cd0094306d08c22a7c6a55fa35915aa263c8db1") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTimeBlock - 3600, nTimeBlock, nNonce, nBits, nVersion, genesisReward);
}

struct thread_data {
    CBlock block;
    uint256 powLimit;
};

void *MineBlock(void *threadarg) {
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    CBlock block = my_data->block;
    uint256 powLimit = my_data->powLimit;
    arith_uint256 hashTarget;
    uint256 PoWHash;
    uint32_t endNonce = block.nNonce + 100000000;
    uint32_t nNonce = block.nNonce;

    block.nBits = UintToArith256(powLimit).GetCompact();
    hashTarget.SetCompact(block.nBits);
    if (hashTarget > UintToArith256(powLimit)) {
        printf("FAILED Invalid nBits\n");
        assert(false);
    }
    while (true) {
        // Check if something found
        if (ScanHash(&block, nNonce, &PoWHash, endNonce)) {
            if (UintToArith256(PoWHash) <= hashTarget) {
                block.nNonce = nNonce;
                GetCurvehashHash(&block, &PoWHash);
                // Found a solution
                printf("SUCCESS block.nTime = %u \n", block.nTime);
                printf("SUCCESS block.nBits = 0x%08x \n", block.nBits);
                printf("SUCCESS block.nNonce = %u \n", block.nNonce);
                printf("SUCCESS block.GetHash = %s\n", block.GetHash().ToString().c_str());
                printf("SUCCESS block.hashMerkleRoot: %s\n", block.hashMerkleRoot.ToString().c_str());
                assert(false);
                break;
            }
        }

    }
}

void MineGenesisBlock(const CBlock& genesis, uint256 powLimit)
{
    const int NUM_THREADS = 8;

    pthread_t threads[NUM_THREADS];
    struct thread_data td[NUM_THREADS];
    pthread_attr_t attr;
    void *status;
    int rc;
    int i = 0;

    // Initialize and set thread joinable
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for( i = 0; i < NUM_THREADS; i++ ) {
        printf("main() : creating thread %d\n", i);
        td[i].block = CreateGenesisBlock(genesis.nTime, genesis.nNonce + 100000000 * (i - 1), genesis.nBits, 1, 0);;
        td[i].powLimit = powLimit;
        rc = pthread_create(&threads[i], &attr, MineBlock, (void *)&td[i]);

        if (rc) {
            printf("Error:unable to create thread\n");
            exit(-1);
        }
    }

    // free attribute and wait for the other threads
    pthread_attr_destroy(&attr);
    for( i = 0; i < NUM_THREADS; i++ ) {
        rc = pthread_join(threads[i], &status);
    }
}
