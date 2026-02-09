// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Copyright (c) 2017-2026 The Meowcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MEOWCOIN_PRIMITIVES_BLOCK_H
#define MEOWCOIN_PRIMITIVES_BLOCK_H

#include <auxpow.h>
#include <primitives/pureheader.h>
#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>
#include <util/time.h>

#include <memory>

// Activation times for PoW algorithm switches
extern uint32_t nKAWPOWActivationTime;
extern uint32_t nMEOWPOWActivationTime;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader : public CPureBlockHeader
{
public:
    // KawPow/MeowPow extended fields
    uint32_t nHeight{0};     // Block height (needed for KawPow DAG epoch)
    uint64_t nNonce64{0};    // Extended nonce for KawPow
    uint256 mix_hash;        // KawPow mix hash

    // Auxpow (if this is a merge-mined block)
    std::shared_ptr<CAuxPow> auxpow;

    CBlockHeader()
    {
        SetNull();
    }

    SERIALIZE_METHODS(CBlockHeader, obj)
    {
        READWRITE(obj.nVersion, obj.hashPrevBlock, obj.hashMerkleRoot, obj.nTime, obj.nBits);
        if (obj.nTime >= nKAWPOWActivationTime && !obj.nVersion.IsAuxpow()) {
            // KawPow/MeowPow format: nNonce is NOT serialized.
            // Instead, the extended fields follow directly after nBits.
            READWRITE(obj.nHeight, obj.nNonce64, obj.mix_hash);
        } else {
            // Legacy (pre-KawPow) or AuxPow format: standard nNonce field
            READWRITE(obj.nNonce);
            if (obj.nVersion.IsAuxpow()) {
                if constexpr (ser_action.ForRead()) {
                    obj.auxpow = std::make_shared<CAuxPow>();
                }
                assert(obj.auxpow);
                READWRITE(*obj.auxpow);
            } else if constexpr (ser_action.ForRead()) {
                obj.auxpow.reset();
            }
        }
    }

    void SetNull()
    {
        CPureBlockHeader::SetNull();
        nHeight = 0;
        nNonce64 = 0;
        mix_hash.SetNull();
        auxpow.reset();
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    // Primary hash function - returns appropriate hash based on PoW algorithm
    uint256 GetHash() const;

    // KawPow-specific hash functions
    uint256 GetKAWPOWHeaderHash() const;
    uint256 GetMEOWPOWHeaderHash() const;
    uint256 GetHashFull(uint256& mix_hash_out) const;

    // Legacy hash functions for pre-KawPow blocks
    uint256 GetX16RHash() const;
    uint256 GetX16RV2Hash() const;

    NodeSeconds Time() const
    {
        return NodeSeconds{std::chrono::seconds{nTime}};
    }

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    /**
     * Set the block's auxpow (or unset it). This takes care of updating
     * the version accordingly.
     * @param apow Pointer to the auxpow to use or nullptr.
     */
    void SetAuxpow(CAuxPow* apow);
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // Memory-only flags for caching expensive checks
    mutable bool fChecked;                            // CheckBlock()
    mutable bool m_checked_witness_commitment{false}; // CheckWitnessCommitment()
    mutable bool m_checked_merkle_root{false};        // CheckMerkleRoot()

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *(static_cast<CBlockHeader*>(this)) = header;
    }

    SERIALIZE_METHODS(CBlock, obj)
    {
        READWRITE(AsBase<CBlockHeader>(obj), obj.vtx);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
        m_checked_witness_commitment = false;
        m_checked_merkle_root = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        block.auxpow         = auxpow;
        // KawPow fields
        block.nHeight        = nHeight;
        block.nNonce64       = nNonce64;
        block.mix_hash       = mix_hash;
        return block;
    }

    std::string ToString() const;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    /** Historically CBlockLocator's version field has been written to network
     * streams as the negotiated protocol version and to disk streams as the
     * client version, but the value has never been used.
     *
     * Hard-code to the highest protocol version ever written to a network stream.
     * SerParams can be used if the field requires any meaning in the future,
     **/
    static constexpr int DUMMY_VERSION = 70016;

    std::vector<uint256> vHave;

    CBlockLocator() = default;

    explicit CBlockLocator(std::vector<uint256>&& have) : vHave(std::move(have)) {}

    SERIALIZE_METHODS(CBlockLocator, obj)
    {
        int nVersion = DUMMY_VERSION;
        READWRITE(nVersion);
        READWRITE(obj.vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

/**
 * Custom serializer for CBlockHeader that omits the nNonce and mixHash, for use
 * as input to KawPow/MeowPow.
 */
class CKAWPOWInput : private CBlockHeader
{
public:
    explicit CKAWPOWInput(const CBlockHeader& header)
    {
        CBlockHeader::SetNull();
        *static_cast<CBlockHeader*>(this) = header;
    }

    SERIALIZE_METHODS(CKAWPOWInput, obj)
    {
        READWRITE(obj.nVersion);
        READWRITE(obj.hashPrevBlock);
        READWRITE(obj.hashMerkleRoot);
        READWRITE(obj.nTime);
        READWRITE(obj.nBits);
        READWRITE(obj.nHeight);
    }
};

class CMEOWPOWInput : private CBlockHeader
{
public:
    explicit CMEOWPOWInput(const CBlockHeader& header)
    {
        CBlockHeader::SetNull();
        *static_cast<CBlockHeader*>(this) = header;
    }

    SERIALIZE_METHODS(CMEOWPOWInput, obj)
    {
        READWRITE(obj.nVersion);
        READWRITE(obj.hashPrevBlock);
        READWRITE(obj.hashMerkleRoot);
        READWRITE(obj.nTime);
        READWRITE(obj.nBits);
        READWRITE(obj.nHeight);
    }
};

#endif // MEOWCOIN_PRIMITIVES_BLOCK_H
