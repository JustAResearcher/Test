// Expanded stubs for removed asset layer to allow iterative compilation.
#ifndef MEOWCOIN_ASSETS_STUB_H
#define MEOWCOIN_ASSETS_STUB_H

#include <string>
#include <vector>
#include <cstdint>
#include <utility>

class CTransaction;
class CScript;
class CBlockIndex;
class COutPoint;

// AssetType subset used in Meowcoin sources
enum AssetType {
    INVALID = 0,
    ROOT,
    SUB,
    UNIQUE,
    MSGCHANNEL,
    QUALIFIER,
    SUB_QUALIFIER,
    RESTRICTED,
    REISSUE,
    OWNER,
    VOTE,
    NULL_ADD_QUALIFIER
};

enum RestrictedType {
    FREEZE_ADDRESS = 0,
    UNFREEZE_ADDRESS,
    GLOBAL_FREEZE,
    GLOBAL_UNFREEZE
};

enum QualifierType {
    ADD_QUALIFIER = 0,
    REMOVE_QUALIFIER
};

struct CNewAsset {
    std::string strName;
    int64_t nAmount{0};
    CNewAsset() = default;
    CNewAsset(const std::string& n, int64_t a): strName(n), nAmount(a) {}
};

struct CAssetTransfer {
    std::string strName;
    int64_t nAmount{0};
    CAssetTransfer() = default;
    CAssetTransfer(const std::string& n, int64_t a): strName(n), nAmount(a) {}
    void ConstructTransaction(CScript&) const {}
};

struct CReissueAsset { };
struct CNullAssetTxData { std::string asset_name; int flag{0}; };
struct CDatabasedAssetData { };

template<typename K, typename V>
class CLRUCache { public: CLRUCache(size_t){ } };

class CAssetsDB { public: bool LoadAssets() { return true;} bool ReadReissuedMempoolState(){return true;} bool ReadBlockUndoAssetData(const uint256&, std::vector<int>&){return true;} bool WriteBlockUndoAssetData(const uint256&, const std::vector<int>&){return true;} void WriteReissuedMempoolState(){} };

class CAssetsCache {
public:
    bool ContainsAsset(const std::string&) const { return false; }
    bool RemoveNewAsset(const CNewAsset&, const std::string&) { return true; }
    bool RemoveOwnerAsset(const std::string&, const std::string&) { return true; }
    bool RemoveReissueAsset(const CReissueAsset&, const std::string&, const std::string&) { return true; }
    bool RemoveTransfer(const CAssetTransfer&, const std::string&, const COutPoint&) { return true; }
    bool RemoveQualifierAddress(const std::string&, const std::string&, QualifierType) { return true; }
    bool RemoveRestrictedAddress(const std::string&, const std::string&, RestrictedType) { return true; }
    bool RemoveGlobalRestricted(const std::string&, RestrictedType) { return true; }
    size_t DynamicMemoryUsage() const { return 0; }
    size_t GetCacheSizeV2() const { return 0; }
    void Flush() {}
};

extern CAssetsDB* passetsdb;
extern CAssetsCache* passets;
extern CLRUCache<std::string, CDatabasedAssetData>* passetsCache;
extern CLRUCache<std::string, CNullAssetTxData>* passetsVerifierCache;
extern CLRUCache<std::string, int8_t>* passetsQualifierCache;
extern CLRUCache<std::string, int8_t>* passetsRestrictionCache;
extern CLRUCache<std::string, int8_t>* passetsGlobalRestrictionCache;

// Helper stubs used in various places
inline bool AssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool AssetFromScript(const CScript&, CNewAsset&, std::string&) { return false; }
inline bool OwnerAssetFromScript(const CScript&, std::string&, std::string&) { return false; }
inline bool ReissueAssetFromScript(const CScript&, CReissueAsset&, std::string&) { return false; }
inline bool MsgChannelAssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool TransferAssetFromScript(const CScript&, CAssetTransfer&, std::string&) { return false; }
inline bool GetAssetData(const CScript&, CDatabasedAssetData&) { return false; }
inline bool ContextualCheckVerifierString(CAssetsCache*, const std::string&, const std::string&, std::string&) { return true; }
inline bool ContextualCheckVerifierString(CAssetsCache*, const std::string&, const std::string&) { return true; }

inline bool IsAssetNameValid(const std::string&, AssetType& t) { t = INVALID; return false; }
inline bool IsAssetNameAnOwner(const std::string&) { return false; }
inline bool IsAssetNameAnMsgChannel(const std::string&) { return false; }
inline bool IsAssetNameAQualifier(const std::string&) { return false; }
inline bool IsAssetNameAnRestricted(const std::string&) { return false; }

inline std::string GetBurnAddress(AssetType) { return std::string(); }
inline int64_t GetBurnAmount(AssetType) { return 0; }
inline std::string GetUniqueAssetName(const std::string&, const std::string&) { return std::string(); }
inline std::string RestrictedNameToOwnerName(const std::string& s) { return s; }
inline std::string DecodeAssetData(const std::string& s) { return s; }

// Fallback to avoid compile dependency
using uint256 = std::array<unsigned char,32>;

#endif // MEOWCOIN_ASSETS_STUB_H
