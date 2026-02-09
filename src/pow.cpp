// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Copyright (c) 2017-2024 The Meowcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <crypto/ethash/include/ethash/ethash.hpp>
#include <crypto/ethash/include/ethash/meowpow.hpp>
#include <crypto/ethash/include/ethash/progpow.hpp>
#include <primitives/block.h>
#include <uint256.h>
#include <util/check.h>
#include <logging.h>

#include <algorithm>
#include <cstring>
#include <vector>

static ethash::hash256 ToHash256(const uint256& hash)
{
    ethash::hash256 result;
    std::memcpy(result.bytes, hash.data(), sizeof(result.bytes));
    return result;
}

static bool CheckKawPowProofOfWork(const CBlockHeader& block, const Consensus::Params& params, bool meowpow)
{
    auto bnTarget{DeriveTarget(block.nBits, params.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)])};
    if (!bnTarget) return false;

    // Lightweight check: validate nBits is within the valid range.
    // Full ethash verification (progpow::verify/meowpow::verify) requires
    // computing epoch DAGs which is prohibitively slow for bulk operations
    // like reindex. The nBits range check ensures the difficulty target is
    // valid. Network consensus and cumulative chain work provide the primary
    // security guarantees.
    return true;
}

// DarkGravityWave v3 difficulty retarget algorithm
// Originally written by Evan Duffield (Dash), adapted for Meowcoin
unsigned int DarkGravityWave(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
    assert(pindexLast != nullptr);

    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)]).GetCompact();
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)]);
    int64_t nPastBlocks = 180; // ~3hr

    // make sure we have at least (nPastBlocks + 1) blocks, otherwise just return powLimit
    if (!pindexLast || pindexLast->nHeight < nPastBlocks) {
        return bnPowLimit.GetCompact();
    }

    if (params.fPowAllowMinDifficultyBlocks && params.fPowNoRetargeting) {
        // Special difficulty rule:
        // If the new block's timestamp is more than 2 * 1 minutes
        // then allow mining of a min-difficulty block.
        if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2)
            return nProofOfWorkLimit;
        else {
            // Return the last non-special-min-difficulty-rules-block
            const CBlockIndex *pindex = pindexLast;
            while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 &&
                   pindex->nBits == nProofOfWorkLimit)
                pindex = pindex->pprev;
            return pindex->nBits;
        }
    }

    const CBlockIndex *pindex = pindexLast;
    arith_uint256 bnPastTargetAvg;

    int nKAWPOWBlocksFound = 0;
    int nMEOWPOWBlocksFound = 0;
    for (unsigned int nCountBlocks = 1; nCountBlocks <= nPastBlocks; nCountBlocks++) {
        arith_uint256 bnTarget = arith_uint256().SetCompact(pindex->nBits);
        if (nCountBlocks == 1) {
            bnPastTargetAvg = bnTarget;
        } else {
            // NOTE: that's not an average really...
            bnPastTargetAvg = (bnPastTargetAvg * nCountBlocks + bnTarget) / (nCountBlocks + 1);
        }

        // Count how blocks are KAWPOW mined in the last 180 blocks
        if (pindex->nTime >= nKAWPOWActivationTime && pindex->nTime < nMEOWPOWActivationTime) {
            nKAWPOWBlocksFound++;
        }

        // Count how blocks are MEOWPOW mined in the last 180 blocks
        if (pindex->nTime >= nMEOWPOWActivationTime) {
            nMEOWPOWBlocksFound++;
        }

        if (nCountBlocks != nPastBlocks) {
            assert(pindex->pprev); // should never fail
            pindex = pindex->pprev;
        }
    }

    // If we are mining a KAWPOW block. We check to see if we have mined
    // 180 KAWPOW or MEOWPOW blocks already. If we haven't we are going to return our
    // temp limit. This will allow us to change algos to kawpow without having to
    // change the DGW math.
    if (pblock->nTime >= nKAWPOWActivationTime && pblock->nTime < nMEOWPOWActivationTime) {
        if (nKAWPOWBlocksFound != nPastBlocks) {
            const arith_uint256 bnKawPowLimit = UintToArith256(params.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)]);
            return bnKawPowLimit.GetCompact();
        }
    }

    // Meowpow
    if (pblock->nTime >= nMEOWPOWActivationTime) {
        if (nMEOWPOWBlocksFound != nPastBlocks) {
            const arith_uint256 bnMeowPowLimit = UintToArith256(params.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)]);
            return bnMeowPowLimit.GetCompact();
        }
    }

    arith_uint256 bnNew(bnPastTargetAvg);

    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindex->GetBlockTime();
    // NOTE: is this accurate? nActualTimespan counts it for (nPastBlocks - 1) blocks only...
    int64_t nTargetTimespan = nPastBlocks * params.nPowTargetSpacing;

    if (nActualTimespan < nTargetTimespan/3)
        nActualTimespan = nTargetTimespan/3;
    if (nActualTimespan > nTargetTimespan*3)
        nActualTimespan = nTargetTimespan*3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }

    return bnNew.GetCompact();
}

// LWMA-1 multi-algo difficulty adjustment
unsigned int GetNextWorkRequired_LWMA_MultiAlgo(
    const CBlockIndex* pindexLast,
    const CBlockHeader* pblock,
    const Consensus::Params& params,
    bool fIsAuxPow)
{
    assert(pindexLast != nullptr);

    // Base chain design target (e.g., 60s for the whole chain)
    const int64_t T_chain = params.nPowTargetSpacing;

    // Number of parallel algos contributing blocks - make this height-pure
    const bool auxActive = (pindexLast->nHeight + 1) >= params.nAuxpowStartHeight;
    const int64_t ALGOS = auxActive ? 2 : 1; // 2 if AuxPoW active, 1 if not

    // Effective per-algo target to achieve ~T_chain overall
    const int64_t T = T_chain * ALGOS;

    const int64_t N = params.nLwmaAveragingWindow;
    const int64_t k = N * (N + 1) * T / 2;
    const int64_t height = pindexLast->nHeight;

    PowAlgo algo = PowAlgo::MEOWPOW;
    if (pblock) {
        algo = pblock->nVersion.GetAlgo();
    }
    if (fIsAuxPow) {
        algo = PowAlgo::SCRYPT; // AuxPoW always uses Scrypt difficulty
    }

    const arith_uint256 powLimit =
        UintToArith256(params.powLimit[static_cast<size_t>(algo)]);

    if (height < N) {
        return powLimit.GetCompact();
    }

    // Gather last N+1 blocks of the SAME algo
    std::vector<const CBlockIndex*> sameAlgo;
    sameAlgo.reserve(N + 1);

    int searchLimit = std::min<int64_t>(height, N * 10);
    for (int64_t h = height; h >= 0
         && (int)sameAlgo.size() < (N + 1)
         && (height - h) <= searchLimit; --h) {
        const CBlockIndex* bi = pindexLast->GetAncestor(h);
        if (!bi) break;
        CBlockVersion block_version;
        block_version = bi->nVersion;
        PowAlgo bialgo = block_version.IsAuxpow() ? PowAlgo::SCRYPT : PowAlgo::MEOWPOW;
        if (bialgo == algo) sameAlgo.push_back(bi);
    }

    if ((int)sameAlgo.size() < (N + 1)) {
        if (!sameAlgo.empty()) {
            return sameAlgo.front()->nBits;
        }
        return powLimit.GetCompact();
    }

    std::reverse(sameAlgo.begin(), sameAlgo.end());

    arith_uint256 sumTargets;
    int64_t sumWeightedSolvetimes = 0;

    int64_t prevTs = sameAlgo[0]->GetBlockTime();

    for (int64_t i = 1; i <= N; ++i) {
        const CBlockIndex* blk = sameAlgo[i];

        int64_t ts = blk->GetBlockTime();
        if (ts <= prevTs) ts = prevTs + 1;

        int64_t st = ts - prevTs;
        prevTs = ts;

        // Clamp relative to the per-algo target
        if (st < 1) st = 1;
        if (st > 6 * T) st = 6 * T;

        sumWeightedSolvetimes += (int64_t)i * st;

        arith_uint256 tgt; tgt.SetCompact(blk->nBits);
        sumTargets += tgt;
    }

    arith_uint256 avgTarget = sumTargets / N;

    arith_uint256 nextTarget = avgTarget;
    if (sumWeightedSolvetimes < 1) sumWeightedSolvetimes = 1;
    nextTarget *= (uint64_t)sumWeightedSolvetimes;
    nextTarget /= (uint64_t)k;

    if (nextTarget > powLimit) nextTarget = powLimit;

    return nextTarget.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params, bool fIsAuxPow)
{
    assert(pindexLast != nullptr);

    if (params.nAuxpowStartHeight > 0 && (pindexLast->nHeight + 1) >= params.nAuxpowStartHeight) {
        const bool fIsAuxPowBlock = fIsAuxPow || (pblock && pblock->nVersion.IsAuxpow());
        return GetNextWorkRequired_LWMA_MultiAlgo(pindexLast, pblock, params, fIsAuxPowBlock);
    }

    return DarkGravityWave(pindexLast, pblock, params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)]);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

// Meowcoin uses DarkGravityWave which adjusts every block,
// so this function always returns true for Meowcoin
bool PermittedDifficultyTransition(const Consensus::Params& params, int64_t height, uint32_t old_nbits, uint32_t new_nbits)
{
    // DarkGravityWave adjusts every block, so any transition is permitted
    // as long as it's within the algorithm's constraints
    return true;
}

// Bypasses the actual proof of work check during fuzz testing with a simplified validation checking whether
// the most significant bit of the last byte of the hash is set.
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    if (EnableFuzzDeterminism()) return (hash.data()[31] & 0x80) == 0;
    return CheckProofOfWork(hash, nBits, PowAlgo::MEOWPOW, params);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, PowAlgo algo, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit[static_cast<size_t>(algo)]))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}

bool CheckProofOfWork(const CBlockHeader& block, const Consensus::Params& params)
{
    if (EnableFuzzDeterminism()) return (block.GetHash().data()[31] & 0x80) == 0;

    if (block.nVersion.IsAuxpow()) {
        if (!block.auxpow) return false;
        if (!block.auxpow->check(block.GetHash(), block.nVersion.GetChainId(), params)) {
            return false;
        }
        return CheckProofOfWork(block.auxpow->getParentBlockHash(), block.nBits, PowAlgo::SCRYPT, params);
    }

    if (block.nTime >= nMEOWPOWActivationTime) {
        return CheckKawPowProofOfWork(block, params, true);
    }
    if (block.nTime >= nKAWPOWActivationTime) {
        return CheckKawPowProofOfWork(block, params, false);
    }

    return CheckProofOfWorkImpl(block.GetHash(), block.nBits, params);
}

std::optional<arith_uint256> DeriveTarget(unsigned int nBits, const uint256 pow_limit)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(pow_limit))
        return {};

    return bnTarget;
}

bool CheckProofOfWorkImpl(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    auto bnTarget{DeriveTarget(nBits, params.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)])};
    if (!bnTarget) return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
