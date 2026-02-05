// Expanded stubs for removed asset layer to allow iterative compilation.
#ifndef MEOWCOIN_ASSETS_STUB_H
#define MEOWCOIN_ASSETS_STUB_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <utility>

class CTransaction;
class CScript;
class CBlockIndex;
class COutPoint;
class uint256;

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

// CNullAssetTxVerifierString - used in verifier cache
struct CNullAssetTxVerifierString {
    std::string verifier_string;
    CNullAssetTxVerifierString() = default;
    CNullAssetTxVerifierString(const std::string& s) : verifier_string(s) {}
};

// CMessage - used for messaging system
struct CMessage {
    COutPoint* out{nullptr}; // Use pointer to avoid forward declaration issues
    std::string strName;
    std::string ipfsHash;
    int64_t time{0};
    int64_t nExpiredTime{0};
    int status{0};
    int nBlockHeight{0};

    CMessage() = default;
    bool operator<(const CMessage& rhs) const { return strName < rhs.strName; }
    bool operator==(const CMessage& rhs) const { return strName == rhs.strName; }
};

// CBlockAssetUndo - used for asset undo data
struct CBlockAssetUndo {
    bool fChangedIPFS{false};
    bool fChangedUnits{false};
    std::string strIPFS;
    int32_t nUnits{0};
    int8_t version{0};
    bool fChangedVerifierString{false};
    std::string verifierString;
};

template<typename K, typename V>
class CLRUCache {
public:
    CLRUCache(size_t) {}
    void insert(const K&, const V&) {}
    bool contains(const K&) const { return false; }
    size_t size() const { return 0; }
};

// Database stub classes
class CAssetsDB {
public:
    bool LoadAssets() { return true; }
    bool ReadReissuedMempoolState() { return true; }
    bool ReadBlockUndoAssetData(const uint256&, std::vector<std::pair<std::string, CBlockAssetUndo>>&) { return true; }
    bool WriteBlockUndoAssetData(const uint256&, const std::vector<std::pair<std::string, CBlockAssetUndo>>&) { return true; }
    void WriteReissuedMempoolState() {}
};

class CMessageDB {
public:
    CMessageDB(size_t, bool = false, bool = false) {}
    bool LoadMessages() { return true; }
    bool EraseMessage(const CMessage&) { return true; }
    bool WriteMessage(const CMessage&) { return true; }
};

class CMessageChannelDB {
public:
    CMessageChannelDB(size_t, bool = false, bool = false) {}
    bool LoadChannels() { return true; }
    bool EraseChannel(const std::string&) { return true; }
    bool WriteChannel(const std::string&) { return true; }
};

class CMyRestrictedDB {
public:
    CMyRestrictedDB(size_t, bool = false, bool = false) {}
    bool LoadMyRestrictedAssets() { return true; }
};

class CRestrictedDB {
public:
    CRestrictedDB(size_t, bool = false, bool = false) {}
    bool LoadRestricted() { return true; }
};

class CSnapshotRequestDB {
public:
    CSnapshotRequestDB(size_t, bool = false, bool = false) {}
    bool LoadRequests() { return true; }
};

class CAssetSnapshotDB {
public:
    CAssetSnapshotDB(size_t, bool = false, bool = false) {}
    bool LoadSnapshots() { return true; }
};

class CDistributeSnapshotRequestDB {
public:
    CDistributeSnapshotRequestDB(size_t, bool = false, bool = false) {}
    bool LoadRequests() { return true; }
};

class CAssetsCache {
public:
    std::map<std::pair<std::string, std::string>, int64_t> mapAssetsAddressAmount;

    CAssetsCache() = default;
    CAssetsCache(const CAssetsCache&) = default;
    CAssetsCache& operator=(const CAssetsCache&) = default;

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
    bool DumpCacheToDatabase() { return true; }
    void Flush() {}
};

// Global extern declarations - defined in validation.cpp
extern CAssetsDB* passetsdb;
extern CAssetsCache* passets;
extern CLRUCache<std::string, CDatabasedAssetData>* passetsCache;
extern CLRUCache<std::string, CMessage>* pMessagesCache;
extern CLRUCache<std::string, int>* pMessageSubscribedChannelsCache;
extern CLRUCache<std::string, int>* pMessagesSeenAddressCache;
extern CMessageDB* pmessagedb;
extern CMessageChannelDB* pmessagechanneldb;
extern CMyRestrictedDB* pmyrestricteddb;
extern CRestrictedDB* prestricteddb;
extern CLRUCache<std::string, CNullAssetTxVerifierString>* passetsVerifierCache;
extern CLRUCache<std::string, int8_t>* passetsQualifierCache;
extern CLRUCache<std::string, int8_t>* passetsRestrictionCache;
extern CLRUCache<std::string, int8_t>* passetsGlobalRestrictionCache;
extern CSnapshotRequestDB* pSnapshotRequestDb;
extern CAssetSnapshotDB* pAssetSnapshotDb;
extern CDistributeSnapshotRequestDB* pDistributeSnapshotDb;

// Global messaging flags
extern bool fMessaging;
inline bool AreMessagesDeployed() { return false; }

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

#endif // MEOWCOIN_ASSETS_STUB_H
