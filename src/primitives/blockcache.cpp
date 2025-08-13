// Copyright (c) 2022 The Raptoreum developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/blockcache.h"
#include "primitives/block.h"
#include "flat-database.h"
#include "sync.h"
#include "../util.h"

CCriticalSection cs_pow;

CBlockCache* CBlockCache::instance = nullptr;

CBlockCache& CBlockCache::Instance()
{
    if (CBlockCache::instance == nullptr)
    {
        int blockCacheSize = gArgs.GetArg("-blockhashcache", DEFAULT_BLOCK_CACHE_SIZE);
        bool blockCacheValidate = gArgs.GetArg("-blockcachevalidate", 0) > 0 ? true : false;
        blockCacheSize = blockCacheSize == 0 ? DEFAULT_BLOCK_CACHE_SIZE : blockCacheSize;

        CBlockCache::instance = new CBlockCache(blockCacheSize, blockCacheValidate);
    }
    return *instance;
}

void CBlockCache::DoMaintenance()
{
    LOCK(cs_pow);
    // If cache has grown enough, save it:
    if (cacheMap.size() - nLoadedSize > 100)
    {
        CFlatDB<CBlockCache> flatDb("blockcache.dat", "blockCache");
        flatDb.Dump(*this);
        LogPrintf("BlockCache: Saving...\n");
    }
}

CBlockCache::CBlockCache(int maxSize, bool validate) : unordered_lru_cache<uint256, uint256, std::hash<uint256>>(maxSize),
   nVersion(CURRENT_VERSION),
   nLoadedSize(0),
   bValidate(validate)
{
    if (bValidate) LogPrintf("BlockCache: Validation and auto correction enabled\n");
}

CBlockCache::~CBlockCache()
{
}

void CBlockCache::Clear()
{
   cacheMap.clear();
}

void CBlockCache::CheckAndRemove()
{
}

std::string CBlockCache::ToString() const
{
    std::ostringstream info;
    info << "BlockCache: elements: " << (int)cacheMap.size();
    return info.str();
}
