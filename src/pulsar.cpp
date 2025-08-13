// Copyright (c) 2013-2017 The Bitcoin Core developers
// Copyright (c) 2017-2019 The Peercoin developers
// Copyright (c) 2022 The Pulsar developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pulsar.h>
#include <streams.h>
#include <secp256k1.h>

// PoW based on elliptic curves.
void Pulsar(const CBlockHeader *pblock, uint32_t nNonce, uint256 *phash)
{
    // Write the first 76 bytes of the block header to a SHA256 state.
    CSHA256 hasher;

    // secp256k1 context for PoW
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_pubkey pubkey;

    unsigned char pub[65];
    size_t publen = 65;
    CSHA256 pubHasher;

    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << *pblock;
    assert(ss.size() == 80);
//
    // Calculate initial SHA256 hash of blockheader and nonce
    hasher.Write((unsigned char*)&ss[0], 76);
    hasher.Write((unsigned char*)&nNonce, 4);
    hasher.Finalize((unsigned char*)phash);

    // 8 rounds of secp256k1 and sha256
    for(int round=0; round<8; round++)
    {
        // Assume SHA256 result as private key and compute uncompressed public key
        assert(secp256k1_ec_pubkey_create(ctx, &pubkey, (unsigned char*)phash) == 1);
        assert(secp256k1_ec_pubkey_serialize(ctx, pub, &publen, &pubkey, SECP256K1_EC_UNCOMPRESSED) == 1);

        // Use SHA256 to hash resulting public key
        CSHA256(pubHasher).Write(pub, 65).Finalize((unsigned char*)phash);
    }
    secp256k1_context_destroy(ctx);
}


void GetCurvehashHash(const CBlockHeader *pblock, uint256 *thash)
{
    Pulsar(pblock, pblock->nNonce, thash);
}
