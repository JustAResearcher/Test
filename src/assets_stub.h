// Expanded stubs for removed asset layer to allow iterative compilation.
#ifndef MEOWCOIN_ASSETS_STUB_H
#define MEOWCOIN_ASSETS_STUB_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <utility>

#include "amount.h"  // For CAmount typedef

class CTransaction;
class CScript;
class CBlockIndex;
class COutPoint;
class CTxOut;
class Coin;
class uint256;
class CWallet;
class CCoinControl;
class CWalletTx;
class CReserveKey;
class CMutableTransaction;

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

// Asset tag constants
#define OWNER_TAG "!"
#define UNIQUE_TAG "#"
#define RESTRICTED_CHAR '$'
#define OWNER_UNITS 0

// Asset undo version constant
const int8_t ASSET_UNDO_INCLUDES_VERIFIER_STRING = -1;

// Cache size constant
#define MAX_CACHE_ASSETS_SIZE 2500

struct CNewAsset {
    std::string strName;
    int64_t nAmount{0};
    int8_t units{8};
    int8_t nReissuable{0};
    int8_t nHasIPFS{0};
    std::string strIPFSHash;
    CNewAsset() = default;
    CNewAsset(const std::string& n, int64_t a): strName(n), nAmount(a) {}
    CNewAsset(const std::string& n, int64_t a, int u, int r, int h, const std::string& ipfs)
        : strName(n), nAmount(a), units(u), nReissuable(r), nHasIPFS(h), strIPFSHash(ipfs) {}
    void SetNull() { strName.clear(); nAmount = 0; units = 8; nReissuable = 0; nHasIPFS = 0; strIPFSHash.clear(); }
    bool IsNull() const { return strName.empty(); }
    std::string ToString() const { return strName; }
    void ConstructTransaction(CScript&) const {}
    void ConstructOwnerTransaction(CScript&) const {}
};

struct CAssetTransfer {
    std::string strName;
    int64_t nAmount{0};
    std::string message;
    int64_t nExpireTime{0};
    CAssetTransfer() = default;
    CAssetTransfer(const std::string& n, int64_t a): strName(n), nAmount(a) {}
    CAssetTransfer(const std::string& n, int64_t a, const std::string& m, int64_t e)
        : strName(n), nAmount(a), message(m), nExpireTime(e) {}
    void ConstructTransaction(CScript&) const {}
    void SetNull() { strName.clear(); nAmount = 0; message.clear(); nExpireTime = 0; }
    bool IsNull() const { return strName.empty(); }
};

struct CReissueAsset {
    std::string strName;
    int64_t nAmount{0};
    int8_t nUnits{0};
    int8_t nReissuable{1};
    std::string strIPFSHash;
    CReissueAsset() = default;
    CReissueAsset(const std::string& n, int64_t a, int u, int r, const std::string& ipfs)
        : strName(n), nAmount(a), nUnits(u), nReissuable(r), strIPFSHash(ipfs) {}
    void SetNull() { strName.clear(); nAmount = 0; nUnits = 0; nReissuable = 1; strIPFSHash.clear(); }
    bool IsNull() const { return strName.empty(); }
    void ConstructTransaction(CScript&) const {}
};
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
    CLRUCache(size_t = 0) {}
    void Put(const K&, const V&) {}
    void Erase(const K&) {}
    V Get(const K&) const { return V(); }
    bool Exists(const K&) const { return false; }
    size_t Size() const { return 0; }
    void Clear() {}
    void SetNull() {}
    size_t MaxSize() const { return 0; }
    void SetSize(size_t) {}
    // Legacy method names
    void insert(const K& k, const V& v) { Put(k, v); }
    bool contains(const K& k) const { return Exists(k); }
    size_t size() const { return Size(); }
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
    bool RemoveReissueAsset(const CReissueAsset&, const std::string&, const COutPoint&, const std::vector<std::pair<std::string, CBlockAssetUndo>>&) { return true; }
    bool RemoveTransfer(const CAssetTransfer&, const std::string&, const COutPoint&) { return true; }
    bool RemoveQualifierAddress(const std::string&, const std::string&, QualifierType) { return true; }
    bool RemoveRestrictedAddress(const std::string&, const std::string&, RestrictedType) { return true; }
    bool RemoveGlobalRestricted(const std::string&, RestrictedType) { return true; }
    bool RemoveRestrictedVerifier(const std::string&, const std::string&, bool = false) { return true; }
    size_t DynamicMemoryUsage() const { return 0; }
    size_t GetCacheSizeV2() const { return 0; }

    // Add methods used in coins.cpp
    bool AddNewAsset(const CNewAsset&, const std::string&, int, const uint256&) { return true; }
    bool AddOwnerAsset(const std::string&, const std::string&) { return true; }
    bool AddReissueAsset(const CReissueAsset&, const std::string&, const COutPoint&) { return true; }
    bool AddTransferAsset(const CAssetTransfer&, const std::string&, const COutPoint&, const CTxOut&) { return true; }
    bool AddQualifierAddress(const std::string&, const std::string&, QualifierType) { return true; }
    bool AddRestrictedAddress(const std::string&, const std::string&, RestrictedType) { return true; }
    bool AddGlobalRestricted(const std::string&, RestrictedType) { return true; }
    bool AddRestrictedVerifier(const std::string&, const std::string&) { return true; }
    bool GetAssetMetaDataIfExists(const std::string&, CNewAsset&) const { return false; }
    bool GetAssetMetaDataIfExists(const std::string&, CNewAsset&, int&, uint256&) const { return false; }
    bool GetAssetVerifierStringIfExists(const std::string&, CNullAssetTxVerifierString&) const { return false; }
    bool CheckForAddressRestriction(const std::string&, const std::string&, bool = false) { return false; }
    bool CheckForGlobalRestriction(const std::string&, bool = false) { return false; }
    bool CheckIfAssetExists(const std::string&, bool = true) { return false; }
    bool CheckForAddressQualifier(const std::string&, const std::string&, bool = false) { return false; }
    bool TrySpendCoin(const COutPoint&, const CTxOut&) { return true; }
    bool UndoAssetCoin(const Coin&, const COutPoint&) { return true; }
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

// Returns the current active asset cache (used by Qt and RPC)
inline CAssetsCache* GetCurrentAssetCache() { return passets; }

// Forward declaration for wallet types
struct CAssetOutputEntry;  // defined in wallet/wallet.h

// Helper stubs used in various places
inline bool AssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool OwnerFromTransaction(const CTransaction&, std::string&, std::string&) { return false; }
inline bool UniqueAssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool AssetFromScript(const CScript&, CNewAsset&, std::string&) { return false; }
inline bool OwnerAssetFromScript(const CScript&, std::string&, std::string&) { return false; }
inline bool ReissueAssetFromScript(const CScript&, CReissueAsset&, std::string&) { return false; }
inline bool ReissueAssetFromTransaction(const CTransaction&, CReissueAsset&, std::string&) { return false; }
inline bool MsgChannelAssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool QualifierAssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool RestrictedAssetFromTransaction(const CTransaction&, CNewAsset&, std::string&) { return false; }
inline bool TransferAssetFromScript(const CScript&, CAssetTransfer&, std::string&) { return false; }
inline bool AssetNullDataFromScript(const CScript&, CNullAssetTxData&, std::string&) { return false; }
inline bool GlobalAssetNullDataFromScript(const CScript&, CNullAssetTxData&) { return false; }
inline bool GetAssetData(const CScript&, CDatabasedAssetData&) { return false; }
inline bool GetAssetData(const CScript&, CAssetOutputEntry&) { return false; }
inline bool VerifyNullAssetDataFlag(const int&, std::string&) { return true; }
inline bool CheckNewAsset(const CNewAsset&, std::string&) { return true; }
inline bool CheckReissueAsset(const CReissueAsset&, std::string&) { return true; }
inline bool ContextualCheckNewAsset(CAssetsCache*, const CNewAsset&, std::string&, bool = false) { return true; }
inline bool ContextualCheckReissueAsset(CAssetsCache*, const CReissueAsset&, std::string&, const CTransaction&) { return true; }
inline bool ContextualCheckTransferAsset(CAssetsCache*, const CAssetTransfer&, const std::string&, std::string&) { return true; }
inline bool ContextualCheckUniqueAssetTx(CAssetsCache*, std::string&, const CTransaction&) { return true; }
inline bool ContextualCheckVerifierString(CAssetsCache*, const std::string&, const std::string&, std::string&) { return true; }
inline bool ContextualCheckVerifierString(CAssetsCache*, const std::string&, const std::string&) { return true; }

inline bool IsAssetNameValid(const std::string&) { return false; }
inline bool IsAssetNameValid(const std::string&, AssetType& t) { t = INVALID; return false; }
inline bool IsAssetNameValid(const std::string&, AssetType&, std::string&) { return false; }
inline bool IsAssetNameAnOwner(const std::string&) { return false; }
inline bool IsAssetNameAnMsgChannel(const std::string&) { return false; }
inline bool IsAssetNameAQualifier(const std::string&) { return false; }
inline bool IsAssetNameAnRestricted(const std::string&) { return false; }

inline std::string GetBurnAddress(AssetType) { return std::string(); }
inline int64_t GetBurnAmount(AssetType) { return 0; }
inline std::string GetUniqueAssetName(const std::string&, const std::string&) { return std::string(); }
inline std::string RestrictedNameToOwnerName(const std::string& s) { return s; }
inline std::string DecodeAssetData(const std::string& s) { return s; }
inline std::string EncodeAssetData(const std::string& s) { return s; }

// GetAssetInfoFromScript - used in rpc/misc.cpp
inline bool GetAssetInfoFromScript(const CScript&, std::string&, int64_t&) { return false; }

// CheckVerifierAssetTxOut stubs - used in tx_verify.cpp
inline bool CheckVerifierAssetTxOut(const CTxOut&, std::string&) { return true; }
inline bool ContextualCheckVerifierAssetTxOut(const CTxOut&, CAssetsCache*, std::string&) { return true; }
inline bool ContextualCheckNullAssetTxOut(const CTxOut&, CAssetsCache*, std::string&, std::vector<std::pair<std::string, CNullAssetTxData>>* = nullptr) { return true; }
inline bool ContextualCheckGlobalAssetTxOut(const CTxOut&, CAssetsCache*, std::string&) { return true; }

// IsScript* stubs - used in tx_verify.cpp
inline bool IsScriptNewAsset(const CScript&) { return false; }
inline bool IsScriptNewAsset(const CScript&, int&) { return false; }
inline bool IsScriptNewUniqueAsset(const CScript&) { return false; }
inline bool IsScriptNewUniqueAsset(const CScript&, int&) { return false; }
inline bool IsScriptOwnerAsset(const CScript&) { return false; }
inline bool IsScriptOwnerAsset(const CScript&, int&) { return false; }
inline bool IsScriptReissueAsset(const CScript&) { return false; }
inline bool IsScriptReissueAsset(const CScript&, int&) { return false; }
inline bool IsScriptTransferAsset(const CScript&) { return false; }
inline bool IsScriptTransferAsset(const CScript&, int&) { return false; }
inline bool IsScriptNewMsgChannelAsset(const CScript&) { return false; }
inline bool IsScriptNewMsgChannelAsset(const CScript&, int&) { return false; }
inline bool IsScriptNewQualifierAsset(const CScript&) { return false; }
inline bool IsScriptNewQualifierAsset(const CScript&, int&) { return false; }
inline bool IsScriptNewRestrictedAsset(const CScript&) { return false; }
inline bool IsScriptNewRestrictedAsset(const CScript&, int&) { return false; }

// Asset name helpers - used in coins.cpp
inline std::string GetParentName(const std::string& name) { return name; }

// Messaging helpers - used in coins.cpp
inline bool IsChannelSubscribed(const std::string&) { return false; }
inline void AddChannel(const std::string&) { }

// Forward declarations for wallet types
class COutput;

// Wallet asset balance functions
inline bool GetAllMyAssetBalances(std::map<std::string, std::vector<COutput>>&, std::map<std::string, CAmount>&, int = 0, bool = true) { return false; }
inline bool GetMyAssetBalance(const std::string&, CAmount&, int = 0) { return false; }

// Asset transaction creation stubs - used in Qt dialogs
inline bool CreateAssetTransaction(CWallet*, CCoinControl&, const CNewAsset&, const std::string&, std::pair<int, std::string>&, CWalletTx&, CReserveKey&, CAmount&, std::string* = nullptr) { return false; }
inline bool CreateAssetTransaction(CWallet*, CCoinControl&, const std::vector<CNewAsset>&, const std::string&, std::pair<int, std::string>&, CWalletTx&, CReserveKey&, CAmount&, std::string* = nullptr) { return false; }
inline bool CreateReissueAssetTransaction(CWallet*, CCoinControl&, const CReissueAsset&, const std::string&, std::pair<int, std::string>&, CWalletTx&, CReserveKey&, CAmount&, std::string* = nullptr) { return false; }
inline bool CreateTransferAssetTransaction(CWallet*, CCoinControl&, const std::vector<std::pair<CAssetTransfer, std::string>>&, const std::string&, std::pair<int, std::string>&, CWalletTx&, CReserveKey&, CAmount&, std::string* = nullptr) { return false; }

// Burn amount and address helpers
inline CAmount GetBurnAmount(AssetType) { return 0; }
inline CAmount GetBurnAmount(int) { return 0; }
inline std::string GetBurnAddress(AssetType) { return ""; }
inline std::string GetBurnAddress(int) { return ""; }
inline CAmount GetReissueAssetBurnAmount() { return 0; }
inline CAmount GetIssueAssetBurnAmount() { return 0; }
inline CAmount GetIssueSubAssetBurnAmount() { return 0; }
inline CAmount GetIssueUniqueAssetBurnAmount() { return 0; }

// Value formatting helpers
inline std::string ValueFromAmountString(CAmount amount, int8_t units) { return std::to_string(amount); }

#endif // MEOWCOIN_ASSETS_STUB_H
