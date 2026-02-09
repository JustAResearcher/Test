// Copyright (c) 2017-2024 The Meowcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <hash_x16r.h>
#include <primitives/block.h>
#include <crypto/ethash/include/ethash/ethash.hpp>
#include <crypto/ethash/include/ethash/progpow.hpp>
#include <crypto/ethash/include/ethash/meowpow.hpp>
#include <crypto/ethash/helpers.hpp>

#include <cstring>

// Global statistics for X16R algorithm selection (debugging)
double algoHashTotal[16] = {0};
int algoHashHits[16] = {0};

// Activation time globals - defined in primitives/block.cpp
extern uint32_t nKAWPOWActivationTime;
extern uint32_t nMEOWPOWActivationTime;

uint256 KAWPOWHash(const CBlockHeader& blockHeader, uint256& mix_hash)
{
    static ethash::epoch_context_ptr context{nullptr, nullptr};

    // Get the context from the block height
    const auto epoch_number = ethash::get_epoch_number(blockHeader.nHeight);

    if (!context || context->epoch_number != epoch_number)
        context = ethash::create_epoch_context(epoch_number);

    // Build the header_hash (convert via hex string to handle byte ordering)
    uint256 nHeaderHash = blockHeader.GetKAWPOWHeaderHash();
    const auto header_hash = to_hash256(nHeaderHash.GetHex());

    // ProgPow hash
    const auto result = progpow::hash(*context, blockHeader.nHeight, header_hash, blockHeader.nNonce64);

    mix_hash = uint256::FromHex(to_hex(result.mix_hash)).value();
    return uint256::FromHex(to_hex(result.final_hash)).value();
}

uint256 KAWPOWHash_OnlyMix(const CBlockHeader& blockHeader)
{
    // Build the header_hash (convert via hex string to handle byte ordering)
    uint256 nHeaderHash = blockHeader.GetKAWPOWHeaderHash();
    const auto header_hash = to_hash256(nHeaderHash.GetHex());

    // Use hash_no_verify: computes final_hash from header_hash + stored mix_hash + nonce
    // without doing the full DAG lookup
    const auto result = progpow::hash_no_verify(blockHeader.nHeight, header_hash,
        to_hash256(blockHeader.mix_hash.GetHex()), blockHeader.nNonce64);

    return uint256::FromHex(to_hex(result)).value();
}

uint256 MEOWPOWHash(const CBlockHeader& blockHeader, uint256& mix_hash)
{
    static ethash::epoch_context_ptr context{nullptr, nullptr};

    // Get the context from the block height
    const auto epoch_number = ethash::get_epoch_number(blockHeader.nHeight);

    if (!context || context->epoch_number != epoch_number)
        context = ethash::create_epoch_context(epoch_number);

    // Build the header_hash (convert via hex string to handle byte ordering)
    uint256 nHeaderHash = blockHeader.GetMEOWPOWHeaderHash();
    const auto header_hash = to_hash256(nHeaderHash.GetHex());

    // MeowPow hash
    const auto result = meowpow::hash(*context, blockHeader.nHeight, header_hash, blockHeader.nNonce64);

    mix_hash = uint256::FromHex(to_hex(result.mix_hash)).value();
    return uint256::FromHex(to_hex(result.final_hash)).value();
}

uint256 MEOWPOWHash_OnlyMix(const CBlockHeader& blockHeader)
{
    // Build the header_hash (convert via hex string to handle byte ordering)
    uint256 nHeaderHash = blockHeader.GetMEOWPOWHeaderHash();
    const auto header_hash = to_hash256(nHeaderHash.GetHex());

    // Use hash_no_verify: computes final_hash from header_hash + stored mix_hash + nonce
    // without doing the full DAG lookup
    const auto result = meowpow::hash_no_verify(blockHeader.nHeight, header_hash,
        to_hash256(blockHeader.mix_hash.GetHex()), blockHeader.nNonce64);

    return uint256::FromHex(to_hex(result)).value();
}
