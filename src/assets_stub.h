// Minimal stubs for removed asset layer to allow incremental compilation.
#ifndef MEOWCOIN_ASSETS_STUB_H
#define MEOWCOIN_ASSETS_STUB_H

#include <string>
#include <vector>

class CTransaction;
class CScript;
class CBlockIndex;

// Minimal asset types and forward declarations used across the codebase.
enum AssetType {
    ASSET_INVALID = 0,
    ASSET_REISSUE = 1,
    ASSET_TRANSFER = 2,
    ASSET_SUB = 3,
    ASSET_UNIQUE = 4,
    ASSET_MSGCHANNEL = 5,
    ASSET_QUALIFIER = 6,
    ASSET_SUB_QUALIFIER = 7,
    ASSET_RESTRICTED = 8
};

struct CNewAsset {
    std::string strName;
    int64_t nAmount = 0;
};

struct CAssetTransfer {
    std::string strName;
    int64_t nAmount = 0;
    void ConstructTransaction(CScript&) const {}
};

class CAssetsCache {
public:
    bool ContainsAsset(const std::string&) const { return false; }
    bool RemoveNewAsset(const CNewAsset&, const std::string&) { return true; }
    size_t DynamicMemoryUsage() const { return 0; }
    size_t GetCacheSizeV2() const { return 0; }
};

// Lightweight helpers (return safe defaults).
inline bool AssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool TransferAssetFromScript(const CScript&, CAssetTransfer&, std::string&) { return false; }
inline bool ParseAssetScript(const CScript&, std::vector<unsigned char>&, std::string&, int64_t&) { return false; }
inline bool IsAssetNameValid(const std::string&, int) { return false; }

#endif // MEOWCOIN_ASSETS_STUB_H
