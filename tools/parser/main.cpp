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
#include "block_replayer.hpp"
#include "address_writer.hpp"
#include "utxo_address_state.hpp"

#include <internal/bitcoin_uint256_hex.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <clipp.h>

#include <wjfilesystem/path.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <fstream>
#include <future>
#include <iostream>
#include <iomanip>
#include <cassert>

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

template <typename BlockType>
struct ChainUpdateInfo {
    std::vector<BlockType> blocksToAdd;
    uint32_t splitPoint;
};

struct StartingCounts {
    uint32_t txCount;
    uint64_t inputCount;
    uint64_t outputCount;
};

uint32_t getStartingCounts(const blocksci::DataConfiguration &config) {
    blocksci::ChainAccess chain{config.chainDirectory(), config.blocksIgnored, config.errorOnReorg};
    if (chain.blockCount() > 0) {
        auto lastBlock = chain.getBlock(chain.blockCount() - 1);
        return lastBlock->firstTxIndex + lastBlock->txCount;
    } else {
        return 0;
    }
}
uint32_t getStartingTxCount(const blocksci::DataConfiguration &config) {
    blocksci::ChainAccess chain{config.chainDirectory(), config.blocksIgnored, config.errorOnReorg};
    if (chain.blockCount() > 0) {
        auto lastBlock = chain.getBlock(chain.blockCount() - 1);
        return lastBlock->firstTxIndex + lastBlock->txCount;
    } else {
        return 0;
    }
}

template <typename ParserTag>
std::vector<blocksci::RawBlock> updateChain(const ParserConfiguration<ParserTag> &config, blocksci::BlockHeight maxBlockNum, HashIndexCreator &hashDb) {
    using namespace std::chrono_literals;
    
    auto chainBlocks = [&]() {
        ChainIndex<ParserTag> index;
        std::ifstream inFile(config.blockListPath().str(), std::ios::binary);
        if (inFile.good()) {
            try {
                boost::archive::binary_iarchive ia(inFile);
                ia >> index;
            } catch (const std::exception &) {
                std::cout << "Error loading chain index. Reparsing from scratch\n";
                index = ChainIndex<ParserTag>{};
            }
        }
        
        index.update(config);
        auto blocks = index.generateChain(maxBlockNum);
        std::ofstream of(config.blockListPath().str(), std::ios::binary);
        boost::archive::binary_oarchive oa(of);
        oa << index;
        return blocks;
    }();

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
    
    rollbackTransactions(splitPoint, hashDb, config);
    
    if (blocksToAdd.size() == 0) {
        return {};
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
        while (newTxCount < 10000000 && it != end) {
            newTxCount += it->nTx;
            ++it;
        }
        
        decltype(blocksToAdd) nextBlocks{prev, it};
        
        auto blocks = processor.addNewBlocks(config, nextBlocks, utxoState, utxoAddressState, addressState, utxoScriptState);
        newBlocks.insert(newBlocks.end(), blocks.begin(), blocks.end());
        
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

void updateConfig(filesystem::path &dataDirectory) {
    auto configFile = dataDirectory/"config.ini";
    
    boost::property_tree::ptree rootPTree;
    rootPTree.put("version", blocksci::dataVersion);
    
    std::ofstream configStream{configFile.str()};
    boost::property_tree::write_ini(configStream, rootPTree);
}

int main(int argc, char * argv[]) {
    
    enum class mode {update, updateCore, updateIndexes, updateHashIndex, updateAddressIndex, compactIndexes, help};
    mode selected = mode::help;


    enum class updateMode {
        disk, rpc
    };
    updateMode selectedUpdateMode = updateMode::disk;
    
    std::string dataDirectoryString;

    auto outputDirOpt = (clipp::required("--output-directory", "-o") & clipp::value("output directory", dataDirectoryString)) % "Path to output parsed data";

    std::string username;
    std::string password;
    std::string address = "127.0.0.1";
    int port = 9998;
    auto rpcOptions = (
        clipp::command("rpc").set(selectedUpdateMode, updateMode::rpc),
        (clipp::required("--username") & clipp::value("username", username)) % "RPC username",
        (clipp::required("--password") & clipp::value("password", password)) % "RPC password",
        (clipp::option("--address") & clipp::value("address", address)) % "RPC address",
        (clipp::option("--port") & clipp::value("port", port)) % "RPC port"
    ).doc("RPC options");

    std::string bitcoinDirectoryString;
    auto fileOptions = (
        clipp::command("disk").set(selectedUpdateMode, updateMode::disk),
        (clipp::required("--coin-directory", "-c") & clipp::value("coin directory", bitcoinDirectoryString)) % "Path to cryptocurrency directory"
    ).doc("File parser options");

    auto updateCommand = clipp::command("update").set(selected,mode::update) % "Update all BlockSci data";
    auto updateCoreCommand = clipp::command("core-update").set(selected,mode::updateCore) % "Update just the core BlockSci data (excluding indexes)";
    auto indexUpdateCommand = clipp::command("index-update").set(selected,mode::updateIndexes) % "Update indexes to latest chain state";
    auto addressIndexUpdateCommand = clipp::command("address-index-update").set(selected,mode::updateAddressIndex) % "Update address index to latest state";
    auto hashIndexUpdateCommand = clipp::command("hash-index-update").set(selected,mode::updateHashIndex) % "Update hash index to latest state";
    auto compactIndexesCommand = clipp::command("compact-indexes").set(selected, mode::compactIndexes) % "Compact indexes to speed up blockchain construction";
    
    int maxBlockNum = 0;
    auto maxBlockOpt = (clipp::option("--max-block", "-m") & clipp::value("max block", maxBlockNum)) % "Max block height to scan up to";
    
    auto coreUpdateOptions = (maxBlockOpt, (fileOptions | rpcOptions));
    
    auto commands = ((updateCommand | updateCoreCommand), coreUpdateOptions) | indexUpdateCommand | addressIndexUpdateCommand | hashIndexUpdateCommand | compactIndexesCommand;
    
    auto cli = (outputDirOpt, commands);
    
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

    filesystem::path dataDirectory = {dataDirectoryString};
//    dataDirectory = dataDirectory.make_absolute();

    if(!dataDirectory.exists()){
        filesystem::create_directory(dataDirectory);
    }

    switch (selected) {
        case mode::update:
        case mode::updateCore: {
            ParserConfigurationBase config{dataDirectory.str()};
            HashIndexCreator hashDb(config, config.dataConfig.hashIndexFilePath());
            std::vector<blocksci::RawBlock> newBlocks;
            switch (selectedUpdateMode) {
                case updateMode::disk: {
                    filesystem::path bitcoinDirectory = {bitcoinDirectoryString};
                    bitcoinDirectory = bitcoinDirectory.make_absolute();
                    ParserConfiguration<FileTag> config{bitcoinDirectory, dataDirectory.str()};
                    newBlocks = updateChain(config, blocksci::BlockHeight{maxBlockNum}, hashDb);
                    break;
                }

                case updateMode::rpc: {
                    ParserConfiguration<RPCTag> config(username, password, address, port, dataDirectory.str());
                    newBlocks = updateChain(config, blocksci::BlockHeight{maxBlockNum}, hashDb);
                    break;
                }
            }
            updateConfig(dataDirectory);
            

            // It'd be nice to do this after the indexes are updated, but they currently depend on the chain being fully updated
            {
                FixedSizeFileWriter<blocksci::RawBlock> blockFile{blocksci::ChainAccess::blockFilePath(config.dataConfig.chainDirectory())};
                for (auto &block : newBlocks) {
                    blockFile.write(block);
                }
            }

            if (selected == mode::update) {
                updateHashDB(config, hashDb);
                updateAddressDB(config);
            }
            
            break;
        }

        case mode::updateIndexes: {
            ParserConfigurationBase config{dataDirectory.str()};
            updateAddressDB(config);
            {
                HashIndexCreator db(config, config.dataConfig.hashIndexFilePath());
                updateHashDB(config, db);
            }
            break;
        }

        case mode::updateHashIndex: {
            ParserConfigurationBase config{dataDirectory.str()};
            HashIndexCreator db(config, config.dataConfig.hashIndexFilePath());
            updateHashDB(config, db);
            break;
        }

        case mode::updateAddressIndex: {
            ParserConfigurationBase config{dataDirectory.str()};
            updateAddressDB(config);
            break;
        }
            
        case mode::compactIndexes: {
            ParserConfigurationBase config{dataDirectory.str()};
            {
                AddressDB db(config, config.dataConfig.addressDBFilePath());
                db.compact();
            }
            {
                HashIndexCreator db(config, config.dataConfig.hashIndexFilePath());
                db.compact();
            }
            break;
        }

        case mode::help: {
            std::cout << clipp::make_man_page(cli, "blocksci_parser");
            break;
        }
    }
    
    return 0;
}
