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
#include <crypto/ethash/include/ethash/ethash.hpp>
#include <crypto/ethash/include/ethash/meowpow.hpp>

// Global activation times - set in chainparams
uint32_t nKAWPOWActivationTime = 0;
uint32_t nMEOWPOWActivationTime = 0;

uint256 CBlockHeader::GetHash() const
{
    // For MeowPow blocks
    if (nTime >= nMEOWPOWActivationTime && nMEOWPOWActivationTime > 0) {
        return GetMEOWPOWHeaderHash();
    }
    // For KawPow blocks  
    if (nTime >= nKAWPOWActivationTime && nKAWPOWActivationTime > 0) {
        return GetKAWPOWHeaderHash();
    }
    // Legacy X16R hash for pre-KawPow blocks
    return GetX16RHash();
}

uint256 CBlockHeader::GetX16RHash() const
{
    // X16R hash - uses previous block hash to select algorithm order
    DataStream ss{};
    ss << nVersion << hashPrevBlock << hashMerkleRoot << nTime << nBits << nNonce;
    return HashX16R(ss.begin(), ss.end(), hashPrevBlock);
}

uint256 CBlockHeader::GetKAWPOWHeaderHash() const
{
    // Create the header hash used as input to KawPow
    return (HashWriter{} << nVersion << hashPrevBlock << hashMerkleRoot << nTime << nBits << nHeight).GetHash();
}

uint256 CBlockHeader::GetMEOWPOWHeaderHash() const
{
    // MeowPow header hash - same structure as KawPow
    return GetKAWPOWHeaderHash();
}

uint256 CBlockHeader::GetHashFull(uint256& mix_hash_out) const
{
    // Compute the full KawPow/MeowPow hash including mix_hash verification
    mix_hash_out = mix_hash;
    return GetHash();
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    if (nTime >= nKAWPOWActivationTime) {
        s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nHeight=%u, nNonce64=%llu, vtx=%u)\n",
            GetHash().ToString(),
            nVersion,
            hashPrevBlock.ToString(),
            hashMerkleRoot.ToString(),
            nTime, nBits, nHeight, nNonce64,
            vtx.size());
    } else {
        s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
            GetHash().ToString(),
            nVersion,
            hashPrevBlock.ToString(),
            hashMerkleRoot.ToString(),
            nTime, nBits, nNonce,
            vtx.size());
    }
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
