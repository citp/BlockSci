//
//  main.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 12/30/15.
//  Copyright © 2015 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "config.hpp"
#include "parser_configuration.hpp"
#include "utxo_state.hpp"
#include "address_state.hpp"
#include "chain_index.hpp"
#include "preproccessed_block.hpp"
#include "block_processor.hpp"
#include "address_db.hpp"
#include "parser_index_creator.hpp"
#include "hash_index_creator.hpp"
#include "address_writer.hpp"
#include "utxo_address_state.hpp"
#include "doctor.hpp"

#include <internal/bitcoin_uint256_hex.hpp>
#include <internal/data_configuration.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <clipp.h>

#include <wjfilesystem/path.h>

#include <cereal/archives/binary.hpp>

#include <nlohmann/json.hpp>

#include <sys/resource.h>

#include <fstream>
#include <future>
#include <iostream>
#include <iomanip>
#include <cassert>

using json = nlohmann::json;

blocksci::State rollbackState(const ParserConfigurationBase &config, blocksci::BlockHeight firstDeletedBlock, uint32_t firstDeletedTxNum) {
    blocksci::State state{
        blocksci::ChainAccess{config.dataConfig.chainDirectory(), config.dataConfig.blocksIgnored, config.dataConfig.errorOnReorg},
        blocksci::ScriptAccess{config.dataConfig.scriptsDirectory()}
    };
    state.blockCount = static_cast<uint32_t>(static_cast<int>(firstDeletedBlock));
    state.txCount = firstDeletedTxNum;
    
    blocksci::IndexedFileMapper<mio::access_mode::write, blocksci::RawTransaction> txFile{blocksci::ChainAccess::txFilePath(config.dataConfig.chainDirectory())};
    blocksci::FixedSizeFileMapper<blocksci::uint256, mio::access_mode::write> txHashesFile{blocksci::ChainAccess::txHashesFilePath(config.dataConfig.chainDirectory())};
    blocksci::ScriptAccess scriptsAccess{config.dataConfig.scriptsDirectory()};
    
    UTXOState utxoState;
    UTXOAddressState utxoAddressState;
    UTXOScriptState utxoScriptState;
    
    utxoAddressState.unserialize(config.utxoAddressStatePath().str());
    utxoState.unserialize(config.utxoCacheFile().str());
    utxoScriptState.unserialize(config.utxoScriptStatePath().str());
    
    auto totalTxCount = static_cast<uint32_t>(txFile.size());
    for (uint32_t txNum = totalTxCount - 1; txNum >= firstDeletedTxNum; txNum--) {
        auto tx = txFile.getData(txNum);
        auto hash = txHashesFile[txNum];
        for (uint16_t i = 0; i < tx->outputCount; i++) {
            auto &output = tx->getOutput(i);
            auto scriptHeader = scriptsAccess.getScriptHeader(output.getAddressNum(), dedupType(output.getType()));
            if (scriptHeader->txFirstSeen == txNum) {
                auto &prevValue = state.scriptCounts.at(static_cast<size_t>(dedupType(output.getType())));
                if (output.getAddressNum() < prevValue) {
                    prevValue = output.getAddressNum();
                }
            }
            if (isSpendable(dedupType(output.getType()))) {
                utxoState.erase({*hash, i});
                utxoAddressState.spendOutput({txNum, i}, output.getType());
                utxoScriptState.erase({txNum, i});
            }
        }
        
        uint32_t inputsAdded = 0;
        for (uint16_t i = 0; i < tx->inputCount; i++) {
            auto &input = tx->getInput(i);
            auto spentTxNum = input.getLinkedTxNum();
            auto spentTx = txFile.getData(spentTxNum);
            auto spentHash = txHashesFile[spentTxNum];
            for (uint16_t j = 0; j < spentTx->outputCount; j++) {
                auto &output = spentTx->getOutput(j);
                if (output.getLinkedTxNum() == txNum) {
                    output.setLinkedTxNum(0);
                    UTXO utxo(output.getValue(), spentTxNum, output.getType());
                    utxoState.add({*spentHash, j}, utxo);
                    blocksci::RawAddress address{output.getAddressNum(), output.getType()};
                    utxoAddressState.addOutput(AnySpendData{address, scriptsAccess}, {spentTxNum, j});
                    utxoScriptState.add({spentTxNum, j}, output.getAddressNum());
                    inputsAdded++;
                }
            }
        }
        assert(inputsAdded == tx->inputCount);
    }
    
    utxoAddressState.serialize(config.utxoAddressStatePath().str());
    utxoState.serialize(config.utxoCacheFile().str());
    utxoScriptState.serialize(config.utxoScriptStatePath().str());
    
    return state;
}

void rollbackTransactions(blocksci::BlockHeight blockKeepCount, HashIndexCreator &hashDb, const ParserConfigurationBase &config) {
    using blocksci::RawBlock;
    using blocksci::IndexedFileMapper;
    using blocksci::SimpleFileMapper;
    using blocksci::FixedSizeFileMapper;
    
    constexpr auto readwrite = mio::access_mode::write;
    blocksci::FixedSizeFileMapper<RawBlock, readwrite> blockFile(blocksci::ChainAccess::blockFilePath(config.dataConfig.chainDirectory()));
    
    if (blockFile.size() > blockKeepCount) {
        
        auto firstDeletedBlock = blockFile[blockKeepCount];
        auto firstDeletedTxNum = firstDeletedBlock->firstTxIndex;
        
        auto blocksciState = rollbackState(config, blockKeepCount, firstDeletedTxNum);
        
        
        IndexedFileMapper<readwrite, blocksci::RawTransaction>(blocksci::ChainAccess::txFilePath(config.dataConfig.chainDirectory())).truncate(firstDeletedTxNum);
        FixedSizeFileMapper<blocksci::uint256, readwrite>(blocksci::ChainAccess::txHashesFilePath(config.dataConfig.chainDirectory())).truncate(firstDeletedTxNum);
        IndexedFileMapper<readwrite, uint32_t>(blocksci::ChainAccess::sequenceFilePath(config.dataConfig.chainDirectory())).truncate(firstDeletedTxNum);
        SimpleFileMapper<readwrite>(blocksci::ChainAccess::blockCoinbaseFilePath(config.dataConfig.chainDirectory())).truncate(firstDeletedBlock->coinbaseOffset);
        blockFile.truncate(blockKeepCount);
        AddressWriter(config).rollback(blocksciState);
        
        AddressState addressState{config.addressPath(), hashDb};
        hashDb.db.rollback(blocksciState.txCount, blocksciState.scriptCounts);
        addressState.reset(blocksciState);
    }
}

void lockDataDirectory(const blocksci::DataConfiguration &dataConfig) {
    filesystem::path pidFile = dataConfig.pidFilePath();

    if(pidFile.exists()) {
        std::cout << "A PID file exists in the data directory, another parser instance might already be running. Aborting." << std::endl;
        exit(1);
    } else {
        std::cout << "Locking data directory." << std::endl;
        std::ofstream rawFile(pidFile.str());
        rawFile << getpid();
        rawFile.close();
    }
}

void lockDataDirectory(const ParserConfigurationBase &config) {
    lockDataDirectory(config.dataConfig);
}

void unlockDataDirectory(const blocksci::DataConfiguration &dataConfig) {
    std::cout << "Unlocking data directory." << std::endl;
    filesystem::path pidFile = dataConfig.pidFilePath();
    if(pidFile.exists()) {
        pidFile.remove_file();
    }
}

void unlockDataDirectory(const ParserConfigurationBase &config) {
    unlockDataDirectory(config.dataConfig);
}

template <typename BlockType>
struct ChainUpdateInfo {
    std::vector<BlockType> blocksToAdd;
    uint32_t splitPoint;
};

template <typename ParserTag>
std::vector<blocksci::RawBlock> updateChain(const ParserConfiguration<ParserTag> &config, blocksci::BlockHeight maxBlockNum, HashIndexCreator &hashDb) {
    using namespace std::chrono_literals;

    /* Load and update the persisted (serialized) ChainIndex object that contains information about all blocks (without transaction data)
     * Generate the ChainIndex if no data is available on disk.
     *
     * This step represents the "xx.x% done fetching block headers" step of the parser output messages.
     */
    auto chainBlocks = [&]() {
        ChainIndex<ParserTag> index;
        std::ifstream inFile(config.blockListPath().str(), std::ios::binary);
        if (inFile.good()) {
            try {
                cereal::BinaryInputArchive ia(inFile);
                ia(index);
            } catch (const std::exception &) {
                std::cout << "Error loading chain index. Reparsing from scratch\n";
                index = ChainIndex<ParserTag>{};
            }
        }
        
        index.update(config, maxBlockNum);
        auto blocks = index.generateChain(maxBlockNum);
        std::ofstream of(config.blockListPath().str(), std::ios::binary);
        cereal::BinaryOutputArchive oa(of);
        oa(index);
        return blocks;
    }();

    /* Determine whether blocks have to be removed from the old chain (due to a fork, eg. caused by miners)
     *
     * This step represents the "Starting with chain of X blocks" and "Adding X blocks" step of the parser output messages.
     */
    blocksci::BlockHeight splitPoint = [&]() {
        blocksci::ChainAccess oldChain{config.dataConfig.chainDirectory(), config.dataConfig.blocksIgnored, config.dataConfig.errorOnReorg};
        blocksci::BlockHeight maxSize = std::min(oldChain.blockCount(), static_cast<blocksci::BlockHeight>(chainBlocks.size()));
        auto splitPoint = maxSize;
        for (blocksci::BlockHeight i{0}; i < maxSize; i++) {
            blocksci::uint256 oldHash = oldChain.getBlock(maxSize - 1 - i)->hash;
            blocksci::uint256 newHash = chainBlocks[static_cast<size_t>(static_cast<int>(maxSize - 1 - i))].hash;
            if (!(oldHash == newHash)) {
                splitPoint = maxSize - 1 - i;
                break;
            }
        }
        
        std::cout << "Starting with chain of " << oldChain.blockCount() << " blocks" << std::endl;
        std::cout << "Removing " << static_cast<blocksci::BlockHeight>(oldChain.blockCount()) - splitPoint << " blocks" << std::endl;
        std::cout << "Adding " << static_cast<blocksci::BlockHeight>(chainBlocks.size()) - splitPoint << " blocks" << std::endl;
        
        return splitPoint;
    }();

    std::vector<BlockInfo<ParserTag>> blocksToAdd{chainBlocks.begin() + static_cast<int>(splitPoint), chainBlocks.end()};
    
    std::ios::sync_with_stdio(false);

    // If blocks have been removed based on the splitPoint, rollback affected transactions as well
    rollbackTransactions(splitPoint, hashDb, config);
    
    if (blocksToAdd.size() == 0) {
        return {};  // No new blocks since the last update
    }
    
    uint32_t startingTxCount;
    uint64_t startingInputCount;
    uint64_t startingOutputCount;
    {
        blocksci::ChainAccess chain{config.dataConfig.chainDirectory(), 0, false};
        startingTxCount = static_cast<uint32_t>(chain.txCount());
        startingInputCount = chain.inputCount();
        startingOutputCount = chain.outputCount();
    }
    
    auto maxBlockHeight = blocksToAdd.back().height;
    
    uint32_t totalTxCount = 0;
    uint32_t totalInputCount = 0;
    uint32_t totalOutputCount = 0;
    for (auto &block : blocksToAdd) {
        totalTxCount += block.nTx;
        totalInputCount += block.inputCount;
        totalOutputCount += block.outputCount;
    }

    BlockProcessor processor{startingTxCount, startingInputCount, startingOutputCount, totalTxCount, maxBlockHeight};
    UTXOState utxoState;
    UTXOAddressState utxoAddressState;
    AddressState addressState{config.addressPath(), hashDb};
    UTXOScriptState utxoScriptState;
    
    utxoAddressState.unserialize(config.utxoAddressStatePath().str());
    utxoState.unserialize(config.utxoCacheFile().str());
    utxoScriptState.unserialize(config.utxoScriptStatePath().str());
    
    std::vector<blocksci::RawBlock> newBlocks;
    auto it = blocksToAdd.begin();
    auto end = blocksToAdd.end();
    while (it != end) {
        auto prev = it;
        uint32_t newTxCount = 0;

        // Process only as many blocks such that ~10,000,000 transactions are added in one BlockProcessor.addNewBlocks() call
        while (newTxCount < 10000000 && it != end) {
            newTxCount += it->nTx;
            ++it;
        }
        
        decltype(blocksToAdd) nextBlocks{prev, it};

        auto blocks = processor.addNewBlocks(config, nextBlocks, utxoState, utxoAddressState, addressState, utxoScriptState);

        // Add all just processed blocks to newBlocks as RawBlock. The blocks are in turn written to blockFile in the calling (parent) function
        newBlocks.insert(newBlocks.end(), blocks.begin(), blocks.end());

        // This step represents the "Back linking transactions" step of the parser output messages.
        backUpdateTxes(config);
    }
    
    utxoAddressState.serialize(config.utxoAddressStatePath().str());
    utxoState.serialize(config.utxoCacheFile().str());
    utxoScriptState.serialize(config.utxoScriptStatePath().str());
    return newBlocks;
}

void updateHashDB(const ParserConfigurationBase &config, HashIndexCreator &db) {
    blocksci::ChainAccess chain{config.dataConfig.chainDirectory(), config.dataConfig.blocksIgnored, config.dataConfig.errorOnReorg};
    blocksci::ScriptAccess scripts{config.dataConfig.scriptsDirectory()};
    
    blocksci::State updateState{chain, scripts};
    std::cout << "Updating hash index\n";
    
    db.runUpdate(updateState);
}

void updateAddressDB(const ParserConfigurationBase &config) {
    blocksci::ChainAccess chain{config.dataConfig.chainDirectory(), config.dataConfig.blocksIgnored, config.dataConfig.errorOnReorg};
    blocksci::ScriptAccess scripts{config.dataConfig.scriptsDirectory()};
    
    blocksci::State updateState{chain, scripts};
    AddressDB db(config, config.dataConfig.addressDBFilePath());
    
    std::cout << "Updating address index\n";
    
    db.runUpdate(updateState);
}

ParserConfigurationBase getBaseConfig(const filesystem::path &configPath) {
    if (!configPath.exists()) {
        throw std::runtime_error("Config path does not exist");
    }
    return {blocksci::loadBlockchainConfig(configPath.str(), true, 0)};
}

void updateChain(const filesystem::path &configFilePath, bool fullParse) {
    auto jsonConf = blocksci::loadConfig(configFilePath.str());
    blocksci::checkVersion(jsonConf);
    
    blocksci::ChainConfiguration chainConfig = jsonConf.at("chainConfig");
    blocksci::DataConfiguration dataConfig{configFilePath.str(), chainConfig, true, 0};
    
    ParserConfigurationBase config{dataConfig};
    HashIndexCreator hashDb(config, config.dataConfig.hashIndexFilePath());
    
    auto parserConf = jsonConf.at("parser");
    blocksci::BlockHeight maxBlock = parserConf.at("maxBlockNum");
    
    std::vector<blocksci::RawBlock> newBlocks;
    if (parserConf.find("disk") != parserConf.end()) {
        ChainDiskConfiguration diskConfig = parserConf.at("disk");
        ParserConfiguration<FileTag> config{dataConfig, diskConfig};
        newBlocks = updateChain(config, blocksci::BlockHeight{maxBlock}, hashDb);
    } else if (parserConf.find("rpc") != parserConf.end()) {
        blocksci::ChainRPCConfiguration rpcConfig = parserConf.at("rpc");
        ParserConfiguration<RPCTag> config(dataConfig, rpcConfig);
        newBlocks = updateChain(config, blocksci::BlockHeight{maxBlock}, hashDb);
    } else {
        throw std::runtime_error("Must provide either rpc or disk parsing settings");
    }
    
    // It'd be nice to do this after the indexes are updated, but they currently depend on the chain being fully updated
    {
        // Write new RawBlock blocks from the updateChain() method to the blockFile
        FixedSizeFileWriter<blocksci::RawBlock> blockFile{blocksci::ChainAccess::blockFilePath(config.dataConfig.chainDirectory())};
        for (auto &block : newBlocks) {
            blockFile.write(block);
        }
    }
    
    if (fullParse) {
        updateHashDB(config, hashDb);
        updateAddressDB(config);
    }
}

int main(int argc, char * argv[]) {
    
    //    blocksci_parser
    //    --config /hkalodner/bitcointest.json
    //    generate-config
    //    --coin-type bitcoin
    //    --data-directory /Users/hkalodner/bitcoin-samp
    //    --coin-directory /Users/hkalodner/Library/Application\ Support/Bitcoin
    
    enum class mode {generateConfig, update, updateCore, updateIndexes, updateHashIndex, updateAddressIndex, compactIndexes, help, doctor};
    mode selected = mode::help;
    
    bool enableRPC = false;
    std::string username;
    std::string password;
    std::string address = "NOTSET";
    int port = -1;
    
    auto rpcOptions = (
        clipp::option("--rpc").set(enableRPC) & (
        clipp::value("username", username) % "RPC username",
        clipp::value("password", password) % "RPC password",
        (clipp::option("--address") & clipp::value("address", address)) % "RPC address",
        (clipp::option("--port") & clipp::value("port", port)) % "RPC port"
    ).doc("RPC options"));
    
    bool enableDisk = false;
    std::string coinDirectoryString;
    auto fileOptions = (
            clipp::option("--disk").set(enableDisk) & (
            clipp::value("coin directory", coinDirectoryString) % "Path to cryptocurrency directory"
        )).doc("File parser options");

    
    std::string coinType;
    int maxBlockNum = 0;
    std::string dataDirectory;
    
    std::stringstream coinTypes;
    coinTypes
    << "coin type (supported modes)\n"
    << "                        bitcoin(disk, rpc)\n"
    << "                        bitcoin_testnet(disk, rpc)\n"
    << "                        bitcoin_regtest(disk, rpc)\n"
    << "                        bitcoin_cash(disk, rpc)\n"
    << "                        bitcoin_cash_testnet(disk, rpc)\n"
    << "                        bitcoin_cash_regtest(disk, rpc)\n"
    << "                        litecoin(disk, rpc)\n"
    << "                        litecoin_testnet(disk, rpc)\n"
    << "                        litecoin_regtest(disk, rpc)\n"
    << "                        dash(rpc)\n"
    << "                        dash_testnet(rpc)\n"
    << "                        namecoin(rpc)\n"
    << "                        namecoin_testnet(rpc)\n"
    << "                        zcash(rpc)\n"
    << "                        zcash_testnet(rpc)\n"
    << "                        custom"
    ;
    auto configOptions = (
          clipp::value("coin type", coinType) % coinTypes.str(),
          clipp::value("data directory", dataDirectory) % "Path to blocksci data location",
          (clipp::option("--max-block", "-m") & clipp::value("max block", maxBlockNum)) % "Max block height to scan up to",
          fileOptions,
          rpcOptions
    ) % "Configuration options";
    
    auto generateConfigCommand = clipp::command("generate-config").set(selected,mode::generateConfig) % "Create new BlockSci configuration";
    auto updateCommand = clipp::command("update").set(selected,mode::update) % "Update all BlockSci data";
    auto updateCoreCommand = clipp::command("core-update").set(selected,mode::updateCore) % "Update just the core BlockSci data (excluding indexes)";
    auto indexUpdateCommand = clipp::command("index-update").set(selected,mode::updateIndexes) % "Update indexes to latest chain state";
    auto addressIndexUpdateCommand = clipp::command("address-index-update").set(selected,mode::updateAddressIndex) % "Update address index to latest state";
    auto hashIndexUpdateCommand = clipp::command("hash-index-update").set(selected,mode::updateHashIndex) % "Update hash index to latest state";
    auto compactIndexesCommand = clipp::command("compact-indexes").set(selected, mode::compactIndexes) % "Compact indexes to speed up blockchain construction";
    auto doctorCommand = clipp::command("doctor").set(selected,mode::doctor) % "Diagnose issues with BlockSci or the provided config file.";
    
    std::string configFilePathString;
    auto configFileOpt = clipp::value("config file", configFilePathString) % "Path to config file";
    
    auto commands = (generateConfigCommand, configOptions) | updateCommand | updateCoreCommand | indexUpdateCommand | addressIndexUpdateCommand | hashIndexUpdateCommand | compactIndexesCommand | doctorCommand;
    
    auto cli = (configFileOpt, commands);
    
    auto res = parse(argc, argv, cli);
    if (res.any_error()) {
//        clipp::debug::print(std::cerr, res);
        for(const auto& m : res.missing()) {
            auto p = m.param();
            std::cout << "missing " << clipp::debug::doc_label(*p) << " after index " << m.after_index() << '\n';
        }
        std::cout << "\n" << clipp::make_man_page(cli, argv[0]);
        return 0;
    }

    filesystem::path configFilePath = {configFilePathString};
    auto folder = configFilePath.parent_path();
    if (!configFilePath.parent_path().exists()) {
        filesystem::create_directory(configFilePath.parent_path());
    }

    switch (selected) {
        case mode::generateConfig: {
            blocksci::ChainConfiguration chainConfig;
            ChainDiskConfiguration diskConfig;
            blocksci::ChainRPCConfiguration rpcConfig;
            
            filesystem::path dataDirectoryPath{dataDirectory};
            if (!dataDirectoryPath.exists()) {
                filesystem::create_directory(dataDirectoryPath);
            }
            
            dataDirectory = dataDirectoryPath.make_absolute().str();
            
            if (enableDisk) {
                coinDirectoryString = filesystem::path{coinDirectoryString}.make_absolute().str();
            }
            
            if (coinType == "bitcoin") {
                chainConfig = blocksci::ChainConfiguration::bitcoin(dataDirectory);
                diskConfig = ChainDiskConfiguration::bitcoin(coinDirectoryString);
                rpcConfig = blocksci::ChainRPCConfiguration::bitcoin(username, password);
            } else if (coinType == "bitcoin_testnet") {
                chainConfig = blocksci::ChainConfiguration::bitcoinTestnet(dataDirectory);
                diskConfig = ChainDiskConfiguration::bitcoinTestnet(coinDirectoryString);
                rpcConfig = blocksci::ChainRPCConfiguration::bitcoinTestnet(username, password);
            } else if (coinType == "bitcoin_regtest") {
                chainConfig = blocksci::ChainConfiguration::bitcoinRegtest(dataDirectory);
                diskConfig = ChainDiskConfiguration::bitcoinRegtest(coinDirectoryString);
            } else if (coinType == "bitcoin_cash") {
                chainConfig = blocksci::ChainConfiguration::bitcoinCash(dataDirectory);
                diskConfig = ChainDiskConfiguration::bitcoinCash(coinDirectoryString);
                rpcConfig = blocksci::ChainRPCConfiguration::bitcoinCash(username, password);
            } else if (coinType == "bitcoin_cash_testnet") {
                chainConfig = blocksci::ChainConfiguration::bitcoinCashTestnet(dataDirectory);
                diskConfig = ChainDiskConfiguration::bitcoinCashTestnet(coinDirectoryString);
                rpcConfig = blocksci::ChainRPCConfiguration::bitcoinCashTestnet(username, password);
            } else if (coinType == "bitcoin_cash_regtest") {
                chainConfig = blocksci::ChainConfiguration::bitcoinCashRegtest(dataDirectory);
                diskConfig = ChainDiskConfiguration::bitcoinCashRegtest(coinDirectoryString);
            } else if (coinType == "dash") {
                chainConfig = blocksci::ChainConfiguration::dash(dataDirectory);
                rpcConfig = blocksci::ChainRPCConfiguration::dash(username, password);
            } else if (coinType == "dash_testnet") {
                chainConfig = blocksci::ChainConfiguration::dashTestnet(dataDirectory);
                rpcConfig = blocksci::ChainRPCConfiguration::dashTestnet(username, password);
            } else if (coinType == "litecoin") {
                chainConfig = blocksci::ChainConfiguration::litecoin(dataDirectory);
                diskConfig = ChainDiskConfiguration::litecoin(coinDirectoryString);
                rpcConfig = blocksci::ChainRPCConfiguration::litecoin(username, password);
            } else if (coinType == "litecoin_testnet") {
                chainConfig = blocksci::ChainConfiguration::litecoinTestnet(dataDirectory);
                diskConfig = ChainDiskConfiguration::litecoinTestnet(coinDirectoryString);
                rpcConfig = blocksci::ChainRPCConfiguration::litecoinTestnet(username, password);
            } else if (coinType == "litecoin_regtest") {
                chainConfig = blocksci::ChainConfiguration::litecoinRegtest(dataDirectory);
                diskConfig = ChainDiskConfiguration::litecoinRegtest(coinDirectoryString);
            } else if (coinType == "namecoin") {
                chainConfig = blocksci::ChainConfiguration::namecoin(dataDirectory);
                rpcConfig = blocksci::ChainRPCConfiguration::namecoin(username, password);
            } else if (coinType == "namecoin_testnet") {
                chainConfig = blocksci::ChainConfiguration::namecoinTestnet(dataDirectory);
                rpcConfig = blocksci::ChainRPCConfiguration::namecoinTestnet(username, password);
            } else if (coinType == "zcash") {
                chainConfig = blocksci::ChainConfiguration::zcash(dataDirectory);
                rpcConfig = blocksci::ChainRPCConfiguration::zcash(username, password);
            } else if (coinType == "zcash_testnet") {
                chainConfig = blocksci::ChainConfiguration::zcashTestnet(dataDirectory);
                rpcConfig = blocksci::ChainRPCConfiguration::zcashTestnet(username, password);
            } else if (coinType == "custom") {
                
            } else {
                std::cout << "Selected invalid coin type\n";
                return 0;
            }
            
            chainConfig.coinName = coinType;
            chainConfig.dataDirectory = dataDirectory;
            rpcConfig.username = username;
            rpcConfig.password = password;
            
            if(address != "NOTSET") {
                rpcConfig.address = address;
            }
            if (port != -1) {
                rpcConfig.port = port;
            }
            
            json parser = json::object({{"maxBlockNum", maxBlockNum}});
            if (enableDisk) {
                parser["disk"] = diskConfig;
            }
            if (enableRPC) {
                parser["rpc"] = rpcConfig;
            }
            
            json jsonConf = {
                {"version", blocksci::dataVersion},
                {"chainConfig", chainConfig},
                {"parser", parser}
            };
            
            std::ofstream rawConf(configFilePath.str());
            rawConf << std::setw(4) << jsonConf;
            
            break;
        }
        case mode::update:
        case mode::updateCore: {
            // Make sure disk space and open files limit are sufficient
            // For more tests run: blocksci_parser <config_file> doctor
            auto doctor = BlockSciDoctor(configFilePath);
            doctor.checkDiskSpace();
            doctor.checkOpenFilesLimit();
            std::cout << std::endl;

            auto config = getBaseConfig(configFilePath);
            lockDataDirectory(config);
            updateChain(configFilePath, selected == mode::update);
            unlockDataDirectory(config);
            break;
        }

        case mode::updateIndexes: {
            auto config = getBaseConfig(configFilePath);
            lockDataDirectory(config);
            updateAddressDB(config);
            {
                HashIndexCreator db(config, config.dataConfig.hashIndexFilePath());
                updateHashDB(config, db);
            }
            unlockDataDirectory(config);
            break;
        }

        case mode::updateHashIndex: {
            auto config = getBaseConfig(configFilePath);
            lockDataDirectory(config);
            HashIndexCreator db(config, config.dataConfig.hashIndexFilePath());
            updateHashDB(config, db);
            unlockDataDirectory(config);
            break;
        }

        case mode::updateAddressIndex: {
            auto config = getBaseConfig(configFilePath);
            lockDataDirectory(config);
            updateAddressDB(config);
            unlockDataDirectory(config);
            break;
        }
            
        case mode::compactIndexes: {
            auto config = getBaseConfig(configFilePath);
            lockDataDirectory(config);
            {
                AddressDB db(config, config.dataConfig.addressDBFilePath());
                db.compact();
            }
            {
                HashIndexCreator db(config, config.dataConfig.hashIndexFilePath());
                db.compact();
            }
            unlockDataDirectory(config);
            break;
        }

        case mode::doctor: {
            auto doctor = BlockSciDoctor(configFilePath);
            doctor.checkDiskSpace();
            doctor.checkOpenFilesLimit();
            std::cout << std::endl;
            doctor.checkConfigFile();
            std::cout << std::endl;
            doctor.rebuildChainIndex();
            std::cout << std::endl;
            doctor.printResults();
            break;
        }

        case mode::help: {
            std::cout << clipp::make_man_page(cli, "blocksci_parser");
            break;
        }
    }
    
    return 0;
}
