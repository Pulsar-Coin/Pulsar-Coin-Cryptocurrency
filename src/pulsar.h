#include <primitives/block.h>
#include <crypto/sha256.h>

// Pulsar PoW
void Pulsar(const CBlockHeader *pblock, uint32_t nNonce, uint256 *phash);

void GetCurvehashHash(const CBlockHeader *pblock, uint256 *thash);