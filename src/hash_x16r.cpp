// Copyright (c) 2017-2024 The Meowcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <hash_x16r.h>
#include <primitives/block.h>
#include <crypto/ethash/include/ethash/ethash.hpp>
#include <crypto/ethash/include/ethash/progpow.hpp>
#include <crypto/ethash/include/ethash/meowpow.hpp>
#include <crypto/ethash/helpers.hpp>

// Global statistics for X16R algorithm selection (debugging)
double algoHashTotal[16] = {0};
int algoHashHits[16] = {0};

// Activation time globals - defined in primitives/block.cpp
extern uint32_t nKAWPOWActivationTime;
extern uint32_t nMEOWPOWActivationTime;

uint256 KAWPOWHash(const CBlockHeader& blockHeader, uint256& mix_hash)
{
    static ethash::epoch_context_ptr context{nullptr, nullptr};

    // Get the header hash (without nNonce64 and mix_hash)
    uint256 header_hash = blockHeader.GetKAWPOWHeaderHash();

    // Build the epoch context if needed
    const auto epoch_number = ethash::get_epoch_number(blockHeader.nHeight);
    if (!context || context->epoch_number != epoch_number)
        context = ethash::create_epoch_context(epoch_number);

    // Compute the KawPow hash
    auto result = progpow::hash(*context, blockHeader.nHeight, 
                                 to_hash256(header_hash), blockHeader.nNonce64);

    mix_hash = IsLessThan(result.mix_hash);
    return IsLessThan(result.final_hash);
}

uint256 KAWPOWHash_OnlyMix(const CBlockHeader& blockHeader)
{
    uint256 mix_hash;
    KAWPOWHash(blockHeader, mix_hash);
    return mix_hash;
}

uint256 MEOWPOWHash(const CBlockHeader& blockHeader, uint256& mix_hash)
{
    static ethash::epoch_context_ptr context{nullptr, nullptr};

    // Get the header hash (without nNonce64 and mix_hash)
    uint256 header_hash = blockHeader.GetMEOWPOWHeaderHash();

    // Build the epoch context if needed
    const auto epoch_number = ethash::get_epoch_number(blockHeader.nHeight);
    if (!context || context->epoch_number != epoch_number)
        context = ethash::create_epoch_context(epoch_number);

    // Compute the MeowPow hash
    auto result = meowpow::hash(*context, blockHeader.nHeight,
                                 to_hash256(header_hash), blockHeader.nNonce64);

    mix_hash = IsLessThan(result.mix_hash);
    return IsLessThan(result.final_hash);
}

uint256 MEOWPOWHash_OnlyMix(const CBlockHeader& blockHeader)
{
    uint256 mix_hash;
    MEOWPOWHash(blockHeader, mix_hash);
    return mix_hash;
}
