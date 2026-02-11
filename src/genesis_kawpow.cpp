// Copyright (c) 2026 The Meowcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash_x16r.h>
#include <crypto/ethash/include/ethash/ethash.hpp>
#include <crypto/ethash/include/ethash/progpow.hpp>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <arith_uint256.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <cstdint>
#include <cstring>
#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

using namespace util::hex_literals;

// Must match the CreateGenesisBlock in kernel/chainparams.cpp exactly
static CBlock CreateGenesisBlock(const char* pszTimestamp,
                                 const CScript& genesisOutputScript,
                                 uint32_t nTime,
                                 uint32_t nNonce,
                                 uint32_t nBits,
                                 int32_t nVersion,
                                 const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.version = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << CScriptNum(0) << 486604799 << CScriptNum(4)
                                       << std::vector<unsigned char>(
                                              reinterpret_cast<const unsigned char*>(pszTimestamp),
                                              reinterpret_cast<const unsigned char*>(pszTimestamp) + std::strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    genesis.nVersion.SetGenesisVersion(nVersion);
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

// Meowcoin-style genesis (matches chainparams.cpp)
static CBlock CreateGenesisBlock(uint32_t nTime,
                                 uint32_t nNonce,
                                 uint32_t nBits,
                                 int32_t nVersion,
                                 const CAmount& genesisReward)
{
    const char* pszTimestamp = "The WSJ 08/28/2022 Investors Ramp Up Bets Against Stock Market";
    const CScript genesisOutputScript = CScript() << "04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f"_hex
                                                  << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

// Testnet4 genesis with unique timestamp
static CBlock CreateTestnet4GenesisBlock(uint32_t nTime,
                                         uint32_t nNonce,
                                         uint32_t nBits,
                                         int32_t nVersion,
                                         const CAmount& genesisReward)
{
    const char* pszTimestamp = "Meowcoin Taproot Testnet 10/Feb/2026";
    const CScript genesisOutputScript = CScript() << "04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f"_hex
                                                  << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

struct GenesisSpec {
    const char* name;
    uint32_t nTime;
    uint32_t nNonce;
    uint32_t nBits;
    int32_t nVersion;
    CAmount reward;
    const char* pow_limit;
};

static ethash::hash256 ToHash256(const uint256& hash)
{
    ethash::hash256 result;
    std::memcpy(result.bytes, hash.data(), sizeof(result.bytes));
    return result;
}

static uint256 ToUint256(const ethash::hash256& hash)
{
    uint256 result;
    std::memcpy(result.data(), hash.bytes, sizeof(hash.bytes));
    return result;
}

static void MineAndPrint(const GenesisSpec& spec)
{
    auto pow_limit = uint256::FromHex(spec.pow_limit);
    assert(pow_limit);

    CBlock genesis;
    if (std::string(spec.name) == "testnet4") {
        genesis = CreateTestnet4GenesisBlock(spec.nTime, spec.nNonce, spec.nBits, spec.nVersion, spec.reward);
    } else {
        genesis = CreateGenesisBlock(spec.nTime, spec.nNonce, spec.nBits, spec.nVersion, spec.reward);
    }
    genesis.nHeight = 0;

    uint256 mix_hash;
    uint256 pow_hash;
    uint64_t nonce = 0;
    bool f_negative = false;
    bool f_overflow = false;
    arith_uint256 target;
    target.SetCompact(genesis.nBits, &f_negative, &f_overflow);
    const bool valid_target = !f_negative && !f_overflow && target != 0 && target <= UintToArith256(*pow_limit);
    const auto start_time = std::chrono::steady_clock::now();

    std::cout << "mining " << spec.name << "..." << std::flush;

    if (!valid_target) {
        std::cout << spec.name << " invalid target for nBits=" << std::hex << genesis.nBits << std::dec << "\n";
        return;
    }

    struct FoundResult {
        uint64_t nonce{0};
        uint256 mix;
        uint256 pow;
    };

    std::atomic<bool> found{false};
    std::atomic<uint64_t> next_nonce{0};
    std::atomic<uint64_t> hashes{0};
    std::mutex found_mutex;
    FoundResult result;

    const unsigned int threads = std::max(1u, std::thread::hardware_concurrency());
    constexpr uint64_t kProgressInterval = 100000;
    std::vector<std::thread> workers;
    workers.reserve(threads);

    for (unsigned int i = 0; i < threads; ++i) {
        workers.emplace_back([&, i]() {
            CBlock work_block = genesis;
            uint256 local_mix;
            uint256 local_pow;
            const auto epoch_number = ethash::get_epoch_number(work_block.nHeight);
            auto context = ethash::create_epoch_context(epoch_number);
            while (!found.load(std::memory_order_acquire)) {
                uint64_t work_nonce = next_nonce.fetch_add(1, std::memory_order_relaxed);
                hashes.fetch_add(1, std::memory_order_relaxed);
                work_block.nNonce64 = work_nonce;
                const ethash::hash256 header_hash = ToHash256(work_block.GetKAWPOWHeaderHash());
                const auto pow_result = progpow::hash(*context, work_block.nHeight, header_hash, work_nonce);
                local_mix = ToUint256(pow_result.mix_hash);
                local_pow = ToUint256(pow_result.final_hash);
                if (UintToArith256(local_pow) <= target) {
                    if (!found.exchange(true, std::memory_order_acq_rel)) {
                        std::lock_guard<std::mutex> lock(found_mutex);
                        result.nonce = work_nonce;
                        result.mix = local_mix;
                        result.pow = local_pow;
                    }
                    break;
                }
                if (i == 0 && (work_nonce % kProgressInterval) == 0) {
                    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::steady_clock::now() - start_time);
                    const double rate = elapsed.count() > 0
                        ? static_cast<double>(hashes.load(std::memory_order_relaxed)) / elapsed.count()
                        : 0.0;
                    std::cout << spec.name << " progress nonce=" << work_nonce
                              << " elapsed=" << elapsed.count() << "s"
                              << " rate=" << static_cast<uint64_t>(rate) << " H/s\n" << std::flush;
                }
            }
        });
    }

    for (auto& t : workers) {
        t.join();
    }

    nonce = result.nonce;
    mix_hash = result.mix;
    pow_hash = result.pow;
    genesis.nNonce64 = nonce;
    genesis.mix_hash = mix_hash;

    const uint256 genesis_hash = genesis.GetHash();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start_time);

    std::cout << spec.name << "\n"
              << "  nonce64: " << nonce << "\n"
              << "  mix_hash: " << genesis.mix_hash.ToString() << "\n"
              << "  pow_hash: " << pow_hash.ToString() << "\n"
              << "  genesis_hash: " << genesis_hash.ToString() << "\n"
              << "  merkle_root: " << genesis.hashMerkleRoot.ToString() << "\n"
              << "  elapsed: " << elapsed.count() << "s\n";
}

int main(int argc, char** argv)
{
    nKAWPOWActivationTime = 0;
    nMEOWPOWActivationTime = std::numeric_limits<uint32_t>::max();

    std::vector<std::string> filter;
    for (int i = 1; i < argc; ++i) {
        filter.emplace_back(argv[i]);
    }

    const GenesisSpec specs[] = {
        {"regtest", 1661730843, 2541049, 0x207fffff, 4, 5000 * COIN,
         "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
        {"testnet", 1661730843, 2541049, 0x1e00ffff, 4, 5000 * COIN,
         "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
        {"signet", 1661730843, 2541049, 0x1e00ffff, 4, 5000 * COIN,
         "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
        {"main", 1661730843, 351574, 0x1e00ffff, 4, 5000 * COIN,
         "00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
        {"testnet4", 1770700000, 0, 0x2000ffff, 4, 5000 * COIN,
         "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
    };

    for (const auto& spec : specs) {
        if (!filter.empty()) {
            bool wanted = false;
            for (const auto& name : filter) {
                if (name == spec.name) {
                    wanted = true;
                    break;
                }
            }
            if (!wanted) {
                continue;
            }
        }
        MineAndPrint(spec);
    }

    return 0;
}
