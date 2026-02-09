// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Copyright (c) 2017-2024 The Meowcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <hash_x16r.h>
#include <streams.h>
#include <tinyformat.h>

// Global activation times - set in chainparams
uint32_t nKAWPOWActivationTime = 0;
uint32_t nMEOWPOWActivationTime = 0;

uint256 CBlockHeader::GetHash() const
{
    if (nVersion.IsAuxpow()) {
        return CPureBlockHeader::GetHash();
    }

    if (nTime < nKAWPOWActivationTime) {
        // All Meowcoin blocks use X16RV2 (X16RV2 activated Oct 2019,
        // before Meowcoin's genesis in Aug 2022)
        return GetX16RV2Hash();
    }

    // For KawPow/MeowPow blocks, the block identity hash is the
    // mix_hash from the PoW computation (not the header hash).
    if (nTime < nMEOWPOWActivationTime) {
        return KAWPOWHash_OnlyMix(*this);
    }
    return MEOWPOWHash_OnlyMix(*this);
}

uint256 CBlockHeader::GetX16RHash() const
{
    // X16R hash - uses raw memory layout (matching original BEGIN(nVersion) END(nNonce))
    const char* pbegin = reinterpret_cast<const char*>(&nVersion);
    const char* pend = reinterpret_cast<const char*>(&nNonce) + sizeof(nNonce);
    return HashX16R(pbegin, pend, hashPrevBlock);
}

uint256 CBlockHeader::GetX16RV2Hash() const
{
    // X16RV2 hash - uses raw memory layout (matching original BEGIN(nVersion) END(nNonce))
    const char* pbegin = reinterpret_cast<const char*>(&nVersion);
    const char* pend = reinterpret_cast<const char*>(&nNonce) + sizeof(nNonce);
    return HashX16RV2(pbegin, pend, hashPrevBlock);
}

uint256 CBlockHeader::GetKAWPOWHeaderHash() const
{
    CKAWPOWInput input{*this};
    HashWriter hasher;
    hasher << input;
    return hasher.GetHash();
}

uint256 CBlockHeader::GetMEOWPOWHeaderHash() const
{
    CMEOWPOWInput input{*this};
    HashWriter hasher;
    hasher << input;
    return hasher.GetHash();
}

uint256 CBlockHeader::GetHashFull(uint256& mix_hash_out) const
{
    if (nTime < nKAWPOWActivationTime) {
        mix_hash_out.SetNull();
        return GetX16RHash();
    }

    if (nTime < nMEOWPOWActivationTime) {
        return KAWPOWHash(*this, mix_hash_out);
    }

    return MEOWPOWHash(*this, mix_hash_out);
}

void CBlockHeader::SetAuxpow(CAuxPow* apow)
{
    if (apow) {
        auxpow.reset(apow);
        nVersion.SetAuxpow(true);
    } else {
        auxpow.reset();
        nVersion.SetAuxpow(false);
    }
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, nNonce64=%u, vtx=%u, auxpow=%s)\n",
        GetHash().ToString(),
        nVersion.GetFullVersion(),
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce, nNonce64,
        vtx.size(),
        auxpow ? auxpow->ToString() : "null");
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
