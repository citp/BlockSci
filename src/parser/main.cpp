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

#include <blocksci/scripts/script_variant.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <clipp.h>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <unordered_set>
#include <future>
#include <iostream>
#include <iomanip>
#include <cassert>

blocksci::State rollbackState(const ParserConfigurationBase &config, blocksci::BlockHeight firstDeletedBlock, uint32_t firstDeletedTxNum) {
    blocksci::State state{blocksci::ChainAccess{config.dataConfig}, blocksci::ScriptAccess{config.dataConfig}};
    state.blockCount = static_cast<uint32_t>(static_cast<int>(firstDeletedBlock));
    state.txCount = firstDeletedTxNum;
    
    blocksci::IndexedFileMapper<blocksci::AccessMode::readwrite, blocksci::RawTransaction> txFile{config.dataConfig.txFilePath()};
    blocksci::FixedSizeFileMapper<blocksci::uint256, blocksci::AccessMode::readwrite> txHashesFile{config.dataConfig.txHashesFilePath()};
    blocksci::DataAccess access(config.dataConfig);
    
    UTXOState utxoState;
    UTXOAddressState utxoAddressState;
    UTXOScriptState utxoScriptState;
    
    utxoAddressState.unserialize(config.utxoAddressStatePath());
    utxoState.unserialize(config.utxoCacheFile().native());
    utxoScriptState.unserialize(config.utxoScriptStatePath().native());
    
    auto totalTxCount = static_cast<uint32_t>(txFile.size());
    for (uint32_t txNum = totalTxCount - 1; txNum >= firstDeletedTxNum; txNum--) {
        auto tx = txFile.getData(txNum);
        auto hash = txHashesFile[txNum];
        for (uint16_t i = 0; i < tx->outputCount; i++) {
            auto &output = tx->getOutput(i);
            blocksci::AnyScript script(output.getAddressNum(), output.getType(), access);
            if (script.firstTxIndex() == txNum) {
                auto &prevValue = state.scriptCounts.at(static_cast<size_t>(dedupType(output.getType())));
                if (output.getAddressNum() < prevValue) {
                    prevValue = output.getAddressNum();
                }
            }
            if (isSpendable(output.getType())) {
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
                    blocksci::AnyScript script(output.getAddressNum(), output.getType(), access);
                    utxoAddressState.addOutput(AnySpendData{script}, {spentTxNum, j});
                    utxoScriptState.add({spentTxNum, j}, output.getAddressNum());
                    inputsAdded++;
                }
            }
        }
        assert(inputsAdded == tx->inputCount);
    }
    
    utxoAddressState.serialize(config.utxoAddressStatePath());
    utxoState.serialize(config.utxoCacheFile().native());
    utxoScriptState.serialize(config.utxoScriptStatePath().native());
    
    return state;
}

void rollbackTransactions(blocksci::BlockHeight blockKeepCount, HashIndexCreator &hashDb, const ParserConfigurationBase &config) {
    using blocksci::AccessMode;
    using blocksci::RawBlock;
    using blocksci::IndexedFileMapper;
    using blocksci::SimpleFileMapper;
    using blocksci::FixedSizeFileMapper;
    
    
    constexpr auto readwrite = AccessMode::readwrite;
    blocksci::FixedSizeFileMapper<RawBlock, readwrite> blockFile(config.dataConfig.blockFilePath());
    
    auto blockKeepSize = static_cast<size_t>(static_cast<int>(blockKeepCount));
    if (blockFile.size() > blockKeepSize) {
        
        auto firstDeletedBlock = blockFile[blockKeepSize];
        auto firstDeletedTxNum = firstDeletedBlock->firstTxIndex;
        
        auto blocksciState = rollbackState(config, blockKeepCount, firstDeletedTxNum);
        
        IndexedFileMapper<readwrite, blocksci::RawTransaction>(config.dataConfig.txFilePath()).truncate(firstDeletedTxNum);
        FixedSizeFileMapper<blocksci::uint256, readwrite>(config.dataConfig.txHashesFilePath()).truncate(firstDeletedTxNum);
        IndexedFileMapper<readwrite, uint32_t>(config.dataConfig.sequenceFilePath()).truncate(firstDeletedTxNum);
        SimpleFileMapper<readwrite>(config.dataConfig.blockCoinbaseFilePath()).truncate(firstDeletedBlock->coinbaseOffset);
        blockFile.truncate(blockKeepSize);
        AddressWriter(config).rollback(blocksciState);
        
        AddressState addressState{config.addressPath(), hashDb};
        hashDb.db.rollback(blocksciState);
        addressState.reset(blocksciState);
    }
}

template <typename BlockType>
struct ChainUpdateInfo {
    std::vector<BlockType> blocksToAdd;
    uint32_t splitPoint;
};

uint32_t getStartingTxCount(const blocksci::DataConfiguration &config) {
    blocksci::ChainAccess chain(config);
    if (chain.blockCount() > 0) {
        auto lastBlock = chain.getBlock(chain.blockCount() - 1);
        return lastBlock->firstTxIndex + lastBlock->numTxes;
    } else {
        return 0;
    }
}

template <typename ParserTag>
std::vector<blocksci::RawBlock> updateChain(const ParserConfiguration<ParserTag> &config, blocksci::BlockHeight maxBlockNum, HashIndexCreator &hashDb) {
    using namespace std::chrono_literals;
    
    auto chainBlocks = [&]() {
        ChainIndex<ParserTag> index;
        boost::filesystem::ifstream inFile(config.blockListPath(), std::ios::binary);
        if (inFile.good()) {
            boost::archive::binary_iarchive ia(inFile);
            ia >> index;
        }
        index.update(config);
        auto blocks = index.generateChain(maxBlockNum);
        boost::filesystem::ofstream of(config.blockListPath(), std::ios::binary);
        boost::archive::binary_oarchive oa(of);
        oa << index;
        return blocks;
    }();

    blocksci::BlockHeight splitPoint = [&]() {
        blocksci::ChainAccess oldChain(config.dataConfig);
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
    
    uint32_t startingTxCount = getStartingTxCount(config.dataConfig);
    auto maxBlockHeight = blocksToAdd.back().height;
    
    uint32_t totalTxCount = 0;
    uint32_t totalInputCount = 0;
    uint32_t totalOutputCount = 0;
    for (auto &block : blocksToAdd) {
        totalTxCount += block.nTx;
        totalInputCount += block.inputCount;
        totalOutputCount += block.outputCount;
    }

    BlockProcessor processor{startingTxCount, totalTxCount, maxBlockHeight};
    UTXOState utxoState;
    UTXOAddressState utxoAddressState;
    AddressState addressState{config.addressPath(), hashDb};
    UTXOScriptState utxoScriptState;
    
    utxoAddressState.unserialize(config.utxoAddressStatePath());
    utxoState.unserialize(config.utxoCacheFile().native());
    utxoScriptState.unserialize(config.utxoScriptStatePath().native());
    
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
    
    utxoAddressState.serialize(config.utxoAddressStatePath());
    utxoState.serialize(config.utxoCacheFile().native());
    utxoScriptState.serialize(config.utxoScriptStatePath().native());
    return newBlocks;
}

void updateHashDB(const ParserConfigurationBase &config, HashIndexCreator &db) {
    blocksci::ChainAccess chain{config.dataConfig};
    blocksci::ScriptAccess scripts{config.dataConfig};
    
    blocksci::State updateState{chain, scripts};
    std::cout << "Updating hash index\n";
    
    db.runUpdate(updateState);
}

void updateAddressDB(const ParserConfigurationBase &config) {
    blocksci::ChainAccess chain{config.dataConfig};
    blocksci::ScriptAccess scripts{config.dataConfig};
    
    blocksci::State updateState{chain, scripts};
    AddressDB db(config, config.dataConfig.addressDBFilePath().native());
    
    std::cout << "Updating address index\n";
    
    db.runUpdate(updateState);
}

void updateConfig(boost::filesystem::path &dataDirectory) {
    auto configFile = dataDirectory/"config.ini";
    
    boost::property_tree::ptree rootPTree;
    rootPTree.put("version", blocksci::dataVersion);
    
    boost::filesystem::ofstream configStream{configFile};
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

    boost::filesystem::path dataDirectory = {dataDirectoryString};
    dataDirectory = boost::filesystem::absolute(dataDirectory);

    if(!(boost::filesystem::exists(dataDirectory))){
        boost::filesystem::create_directory(dataDirectory);
    }

    switch (selected) {
        case mode::update:
        case mode::updateCore: {
            ParserConfigurationBase config{dataDirectory};
            HashIndexCreator hashDb(config, config.dataConfig.hashIndexFilePath().native());
            std::vector<blocksci::RawBlock> newBlocks;
            switch (selectedUpdateMode) {
                case updateMode::disk: {
                    boost::filesystem::path bitcoinDirectory = {bitcoinDirectoryString};
                    bitcoinDirectory = boost::filesystem::absolute(bitcoinDirectory);
                    ParserConfiguration<FileTag> config{bitcoinDirectory, dataDirectory};
                    newBlocks = updateChain(config, blocksci::BlockHeight{maxBlockNum}, hashDb);
                    break;
                }

                case updateMode::rpc: {
                    ParserConfiguration<RPCTag> config(username, password, address, port, dataDirectory);
                    newBlocks = updateChain(config, blocksci::BlockHeight{maxBlockNum}, hashDb);
                    break;
                }
            }
            updateConfig(dataDirectory);
            

            // It'd be nice to do this after the indexes are updated, but they currently depend on the chain being fully updated
            {
                blocksci::FixedSizeFileWriter<blocksci::RawBlock> blockFile{config.dataConfig.blockFilePath()};
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
            ParserConfigurationBase config{dataDirectory};
            updateAddressDB(config);
            {
                HashIndexCreator db(config, config.dataConfig.hashIndexFilePath().native());
                updateHashDB(config, db);
            }
            break;
        }

        case mode::updateHashIndex: {
            ParserConfigurationBase config{dataDirectory};
            HashIndexCreator db(config, config.dataConfig.hashIndexFilePath().native());
            updateHashDB(config, db);
            break;
        }

        case mode::updateAddressIndex: {
            ParserConfigurationBase config{dataDirectory};
            updateAddressDB(config);
            break;
        }
            
        case mode::compactIndexes: {
            ParserConfigurationBase config{dataDirectory};
            {
                AddressDB db(config, config.dataConfig.addressDBFilePath().native());
                db.compact();
            }
            {
                HashIndexCreator db(config, config.dataConfig.hashIndexFilePath().native());
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
