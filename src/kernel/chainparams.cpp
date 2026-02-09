// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <kernel/messagestartchars.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/chaintype.h>
#include <util/strencodings.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

using namespace util::hex_literals;

// Workaround MSVC bug triggering C7595 when calling consteval constructors in
// initializer lists.
// https://developercommunity.visualstudio.com/t/Bogus-C7595-error-on-valid-C20-code/10906093
#if defined(_MSC_VER)
auto consteval_ctor(auto&& input) { return input; }
#else
#define consteval_ctor(input) (input)
#endif

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.version = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << CScriptNum(0) << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion.SetGenesisVersion(nVersion);
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The WSJ 08/28/2022 Investors Ramp Up Bets Against Stock Market";
    const CScript genesisOutputScript = CScript() << "04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f"_hex << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}


/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        m_chain_type = ChainType::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 2100000;  // Meowcoin: ~4 years at 1 min block time
        consensus.script_flag_exceptions.clear(); // Meowcoin: clean chain, no exceptions needed
        consensus.BIP34Height = 1; // Meowcoin: enabled from start
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1; // Meowcoin: enabled from start
        consensus.BIP66Height = 1; // Meowcoin: enabled from start
        consensus.CSVHeight = 1; // Meowcoin: enabled from start
        consensus.SegwitHeight = 0; // Meowcoin: segwit always active (nSegwitEnabled = true in original)
        consensus.MinBIP9WarningHeight = 2016;
        consensus.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)] = uint256{"00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"}; // Meowcoin MeowPow limit
        consensus.powLimit[static_cast<size_t>(PowAlgo::SCRYPT)] = uint256{"00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.nKAWPOWActivationTime = 1662493424; // UTC: Sep 6, 2022 (KawPow activation)
        consensus.nMEOWPOWActivationTime = 1710799200; // March 18, 2024 22:00:00 UTC
        consensus.nPowTargetTimespan = 2016 * 60; // Meowcoin: 1.4 days
        consensus.nPowTargetSpacing = 1 * 60; // Meowcoin: 1 minute blocks
        consensus.nLwmaAveragingWindow = 45;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.enforce_BIP94 = false;
        consensus.fPowNoRetargeting = false;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 1815; // 90%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 2016;

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1788739200; // Sep 7, 2026 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 2115366; // SegWit + 1 month at 60s spacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].threshold = 1815; // 90%
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].period = 2016;

        consensus.nMinimumChainWork = uint256{}; // Disable presync anti-DoS (not needed for Meowcoin)
        consensus.defaultAssumeValid = uint256{}; // Validate all blocks during initial sync
        consensus.nAuxpowChainId = 9;
        consensus.nAuxpowStartHeight = 1614560;
        consensus.fStrictChainId = true;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x4d; // M
        pchMessageStart[1] = 0x45; // E
        pchMessageStart[2] = 0x57; // W
        pchMessageStart[3] = 0x43; // C
        nDefaultPort = 8788; // Meowcoin mainnet port
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 5; // Meowcoin is much smaller than Bitcoin
        m_assumed_chain_state_size = 1;

        // Set globals before genesis hash computation so GetHash() picks the correct PoW algorithm
        nKAWPOWActivationTime = consensus.nKAWPOWActivationTime;
        nMEOWPOWActivationTime = consensus.nMEOWPOWActivationTime;

        // Meowcoin genesis block
        genesis = CreateGenesisBlock(1661730843, 351574, 0x1e00ffff, 4, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256{"000000edd819220359469c54f2614b5602ebc775ea67a64602f354bdaa320f70"});
        assert(genesis.hashMerkleRoot == uint256{"e8916cf6592c8433d598c3a5fe60a9741fd2a997b39d93af2d789cdd9d9a7390"});

        // Meowcoin DNS seeds (from original Meowcoin wallet)
        vSeeds.emplace_back("seed-mainnet-mewc.meowcoin.cc.");
        vSeeds.emplace_back("dnsseed.nodeslist.xyz.");

        // Meowcoin address prefixes
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,50);  // M
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,122); // m
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,112);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "mewc"; // Meowcoin bech32 prefix

        // Fixed seeds disabled - data from Bitcoin Core, not Meowcoin nodes
        // TODO: Generate proper Meowcoin fixed seeds
        vFixedSeeds.clear();

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        // Meowcoin: No assumeutxo data yet
        m_assumeutxo_data = {};

        chainTxData = ChainTxData{
            // Meowcoin chain data
            .nTime    = 1661730843, // Genesis time
            .tx_count = 50000,
            .dTxRate  = 0.5,
        };
    }
};

/**
 * Testnet: Meowcoin test network
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        m_chain_type = ChainType::TESTNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 2100000; // Same as mainnet
        consensus.script_flag_exceptions.clear();
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 0; // Meowcoin: segwit always active
        consensus.MinBIP9WarningHeight = 2016;
        consensus.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.powLimit[static_cast<size_t>(PowAlgo::SCRYPT)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.nKAWPOWActivationTime = 1661833868; // KawPow activation for testnet
        consensus.nMEOWPOWActivationTime = 1707354000; // Feb 4, 2024
        consensus.nPowTargetTimespan = 2016 * 60;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.nLwmaAveragingWindow = 45;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.enforce_BIP94 = false;
        consensus.fPowNoRetargeting = false;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 2016;

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1788739200; // Sep 7, 2026 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 2115366; // SegWit + 1 month at 60s spacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].period = 2016;

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{}; // Meowcoin testnet
        consensus.nAuxpowChainId = 9;
        consensus.nAuxpowStartHeight = 46;
        consensus.fStrictChainId = true;

        // Meowcoin testnet magic bytes
        pchMessageStart[0] = 0x4d; // M
        pchMessageStart[1] = 0x45; // E
        pchMessageStart[2] = 0x57; // W
        pchMessageStart[3] = 0x54; // T (for Testnet)
        nDefaultPort = 4569; // Meowcoin testnet port
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        // Set globals before genesis hash computation so GetHash() picks the correct PoW algorithm
        nKAWPOWActivationTime = consensus.nKAWPOWActivationTime;
        nMEOWPOWActivationTime = consensus.nMEOWPOWActivationTime;

        // Meowcoin testnet genesis
        genesis = CreateGenesisBlock(1661734222, 7680541, 0x1e00ffff, 4, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256{"000000eaab417d6dfe9bd75119972e1d07ecfe8ff655bef7c2acb3d9a0eeed81"});
        assert(genesis.hashMerkleRoot == uint256{"e8916cf6592c8433d598c3a5fe60a9741fd2a997b39d93af2d789cdd9d9a7390"});

        vFixedSeeds.clear();
        vSeeds.clear();
        // Meowcoin testnet seeds
        vSeeds.emplace_back("testnet-seed.meowcoin.net.");

        // Meowcoin testnet address prefixes
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,109); // m
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,124);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,114);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tmewc"; // Meowcoin testnet

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        m_assumeutxo_data = {};

        chainTxData = ChainTxData{
            .nTime    = 1661730843,
            .tx_count = 0,
            .dTxRate  = 0,
        };
    }
};

/**
 * Testnet4 - Not used for Meowcoin
 */
class CTestNet4Params : public CChainParams {
public:
    CTestNet4Params() {
        m_chain_type = ChainType::TESTNET4;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 2100000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 0; // Meowcoin: segwit always active
        consensus.MinBIP9WarningHeight = 2016;
        consensus.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.powLimit[static_cast<size_t>(PowAlgo::SCRYPT)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.nKAWPOWActivationTime = 1661833868; // Same as testnet
        consensus.nMEOWPOWActivationTime = 1707354000;
        consensus.nPowTargetTimespan = 2016 * 60;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.nLwmaAveragingWindow = 45;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.enforce_BIP94 = false;
        consensus.fPowNoRetargeting = false;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 2016;

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1788739200; // Sep 7, 2026 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 2115366; // SegWit + 1 month at 60s spacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].period = 2016;

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};
        consensus.nAuxpowChainId = 9;
        consensus.nAuxpowStartHeight = 46;
        consensus.fStrictChainId = true;

        pchMessageStart[0] = 0x4d; // M
        pchMessageStart[1] = 0x45; // E
        pchMessageStart[2] = 0x57; // W
        pchMessageStart[3] = 0x54; // T (for Testnet)
        nDefaultPort = 4569;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        // Set globals before genesis hash computation so GetHash() picks the correct PoW algorithm
        nKAWPOWActivationTime = consensus.nKAWPOWActivationTime;
        nMEOWPOWActivationTime = consensus.nMEOWPOWActivationTime;

        genesis = CreateGenesisBlock(1661734222, 7680541, 0x1e00ffff, 4, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256{"000000eaab417d6dfe9bd75119972e1d07ecfe8ff655bef7c2acb3d9a0eeed81"});
        assert(genesis.hashMerkleRoot == uint256{"e8916cf6592c8433d598c3a5fe60a9741fd2a997b39d93af2d789cdd9d9a7390"});

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.emplace_back("testnet-seed.meowcoin.net.");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,109); // m
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,124);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,114);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tmewc";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        m_assumeutxo_data = {};

        chainTxData = ChainTxData{
            .nTime    = 1661730843,
            .tx_count = 0,
            .dTxRate  = 0,
        };
    }
};

/**
 * Signet: test network with an additional consensus parameter (see BIP325).
 */
class SigNetParams : public CChainParams {
public:
    explicit SigNetParams(const SigNetOptions& options)
    {
        std::vector<uint8_t> bin;
        vFixedSeeds.clear();
        vSeeds.clear();

        if (!options.challenge) {
            bin = ""_hex_v_u8;
        } else {
            bin = *options.challenge;
        }

        if (options.seeds) {
            vSeeds = *options.seeds;
        } else {
            vSeeds.emplace_back("testnet-seed.meowcoin.net.");
        }

        m_chain_type = ChainType::SIGNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 2100000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 0; // Meowcoin: segwit always active
        consensus.nPowTargetTimespan = 2016 * 60;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.enforce_BIP94 = false;
        consensus.fPowNoRetargeting = false;
        consensus.MinBIP9WarningHeight = 2016;
        consensus.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.powLimit[static_cast<size_t>(PowAlgo::SCRYPT)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.nKAWPOWActivationTime = 1661833868; // Same as testnet
        consensus.nMEOWPOWActivationTime = 1707354000;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 2016;

        // Activation of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1788739200; // Sep 7, 2026 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 2115366; // SegWit + 1 month at 60s spacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].period = 2016;

        pchMessageStart[0] = 0x4d; // M
        pchMessageStart[1] = 0x45; // E
        pchMessageStart[2] = 0x57; // W
        pchMessageStart[3] = 0x54; // T (for Testnet)

        nDefaultPort = 4569;
        nPruneAfterHeight = 1000;

        // Set globals before genesis hash computation so GetHash() picks the correct PoW algorithm
        nKAWPOWActivationTime = consensus.nKAWPOWActivationTime;
        nMEOWPOWActivationTime = consensus.nMEOWPOWActivationTime;

        genesis = CreateGenesisBlock(1661734222, 7680541, 0x1e00ffff, 4, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256{"000000eaab417d6dfe9bd75119972e1d07ecfe8ff655bef7c2acb3d9a0eeed81"});
        assert(genesis.hashMerkleRoot == uint256{"e8916cf6592c8433d598c3a5fe60a9741fd2a997b39d93af2d789cdd9d9a7390"});

        m_assumeutxo_data = {};

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,109); // m
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,124);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,114);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tmewc";

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;
        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;
        chainTxData = ChainTxData{
            .nTime    = 1661730843,
            .tx_count = 0,
            .dTxRate  = 0,
        };
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams(const RegTestOptions& opts)
    {
        m_chain_type = ChainType::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 2100000;
        consensus.BIP34Height = 1; // Always active unless overridden
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1;  // Always active unless overridden
        consensus.BIP66Height = 1;  // Always active unless overridden
        consensus.CSVHeight = 1;    // Always active unless overridden
        consensus.SegwitHeight = 0; // Meowcoin: segwit always active
        consensus.MinBIP9WarningHeight = 2016;
        consensus.powLimit[static_cast<size_t>(PowAlgo::MEOWPOW)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.powLimit[static_cast<size_t>(PowAlgo::SCRYPT)] = uint256{"7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        consensus.nKAWPOWActivationTime = 3582830167; // Far future for regtest
        consensus.nMEOWPOWActivationTime = 3582830167; // Far future for regtest
        consensus.nPowTargetTimespan = 2016 * 60;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.nLwmaAveragingWindow = 45;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.enforce_BIP94 = false;
        consensus.fPowNoRetargeting = false;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 2016;

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1788739200; // Sep 7, 2026 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 2115366; // SegWit + 1 month at 60s spacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].threshold = 1512; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].period = 2016;

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};
        consensus.nAuxpowChainId = 9;
        consensus.nAuxpowStartHeight = 19200;
        consensus.fStrictChainId = true;

        pchMessageStart[0] = 0x44; // D
        pchMessageStart[1] = 0x52; // R
        pchMessageStart[2] = 0x4F; // O
        pchMessageStart[3] = 0x57; // W
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        for (const auto& [dep, height] : opts.activation_heights) {
            switch (dep) {
            case Consensus::BuriedDeployment::DEPLOYMENT_SEGWIT:
                consensus.SegwitHeight = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_HEIGHTINCB:
                consensus.BIP34Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_DERSIG:
                consensus.BIP66Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CLTV:
                consensus.BIP65Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CSV:
                consensus.CSVHeight = int{height};
                break;
            }
        }

        for (const auto& [deployment_pos, version_bits_params] : opts.version_bits_parameters) {
            consensus.vDeployments[deployment_pos].nStartTime = version_bits_params.start_time;
            consensus.vDeployments[deployment_pos].nTimeout = version_bits_params.timeout;
            consensus.vDeployments[deployment_pos].min_activation_height = version_bits_params.min_activation_height;
        }

        // Set globals before genesis hash computation so GetHash() picks the correct PoW algorithm
        nKAWPOWActivationTime = consensus.nKAWPOWActivationTime;
        nMEOWPOWActivationTime = consensus.nMEOWPOWActivationTime;

        genesis = CreateGenesisBlock(1661734578, 1, 0x207fffff, 4, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        // Regtest genesis hash assertions commented out (like original Meowcoin)
        //assert(consensus.hashGenesisBlock == uint256{"..."});
        assert(genesis.hashMerkleRoot == uint256{"e8916cf6592c8433d598c3a5fe60a9741fd2a997b39d93af2d789cdd9d9a7390"});

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));
        vSeeds.clear();
        vSeeds.emplace_back("testnet-seed.meowcoin.net.");

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        m_assumeutxo_data = {};

        chainTxData = ChainTxData{
            .nTime    = 1661730843,
            .tx_count = 0,
            .dTxRate  = 0,
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,109); // m
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,124);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,114);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tmewc";
    }
};

std::unique_ptr<const CChainParams> CChainParams::SigNet(const SigNetOptions& options)
{
    return std::make_unique<const SigNetParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::RegTest(const RegTestOptions& options)
{
    return std::make_unique<const CRegTestParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::Main()
{
    return std::make_unique<const CMainParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet()
{
    return std::make_unique<const CTestNetParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet4()
{
    return std::make_unique<const CTestNet4Params>();
}

std::vector<int> CChainParams::GetAvailableSnapshotHeights() const
{
    std::vector<int> heights;
    heights.reserve(m_assumeutxo_data.size());

    for (const auto& data : m_assumeutxo_data) {
        heights.emplace_back(data.height);
    }
    return heights;
}

std::optional<ChainType> GetNetworkForMagic(const MessageStartChars& message)
{
    const auto mainnet_msg = CChainParams::Main()->MessageStart();
    const auto testnet_msg = CChainParams::TestNet()->MessageStart();
    const auto testnet4_msg = CChainParams::TestNet4()->MessageStart();
    const auto regtest_msg = CChainParams::RegTest({})->MessageStart();
    const auto signet_msg = CChainParams::SigNet({})->MessageStart();

    if (std::ranges::equal(message, mainnet_msg)) {
        return ChainType::MAIN;
    } else if (std::ranges::equal(message, testnet_msg)) {
        return ChainType::TESTNET;
    } else if (std::ranges::equal(message, testnet4_msg)) {
        return ChainType::TESTNET4;
    } else if (std::ranges::equal(message, regtest_msg)) {
        return ChainType::REGTEST;
    } else if (std::ranges::equal(message, signet_msg)) {
        return ChainType::SIGNET;
    }
    return std::nullopt;
}
