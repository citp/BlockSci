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
#include "first_seen_index.hpp"
#include "parser_index_creator.hpp"
#include "hash_index_creator.hpp"
#include "block_replayer.hpp"
#include "address_writer.hpp"

#include <blocksci/data_access.hpp>
#include <blocksci/state.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/script_first_seen_access.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <unordered_set>
#include <future>
#include <iostream>
#include <iomanip>
#include <cassert>

void rollbackTransactions(size_t blockKeepCount, const ParserConfigurationBase &config);
std::vector<char> HexToBytes(const std::string& hex);
uint32_t getStartingTxCount(const blocksci::DataConfiguration &config);


blocksci::State rollbackState(const ParserConfigurationBase &config, uint32_t firstDeletedBlock, uint32_t firstDeletedTxNum) {
    blocksci::State state{blocksci::ChainAccess{config, false, 0}, blocksci::ScriptAccess{config}};
    state.blockCount = firstDeletedBlock;
    state.txCount = firstDeletedTxNum;
    
    blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> txFile{config.txFilePath()};
    blocksci::FixedSizeFileMapper<blocksci::uint256, boost::iostreams::mapped_file::readwrite> txHashesFile{config.txHashesFilePath()};
    blocksci::ScriptFirstSeenAccess firstSeenIndex(config);
    UTXOState utxoState{config};
    
    uint32_t totalTxCount = static_cast<uint32_t>(txFile.size());
    for (uint32_t txNum = totalTxCount - 1; txNum >= firstDeletedTxNum; txNum--) {
        auto tx = txFile.getData(txNum);
        auto hash = txHashesFile.getData(txNum);
        
        for (uint16_t i = 0; i < tx->outputCount; i++) {
            auto &output = tx->getOutput(i);
            if (output.getAddress().getFirstTransactionIndex(firstSeenIndex) == txNum) {
                auto &prevValue = state.scriptCounts[static_cast<size_t>(scriptType(output.getType()))];
                auto addressNum = output.getAddress().addressNum;
                if (addressNum < prevValue) {
                    prevValue = addressNum;
                }
            }
            if (output.linkedTxNum != 0) {
                utxoState.spendOutput({*hash, i});
            }
        }
        
        for (uint16_t i = 0; i < tx->inputCount; i++) {
            auto &input = tx->getInput(i);
            auto spentTxNum = input.spentTxIndex();
            auto spentTx = txFile.getData(spentTxNum);
            auto spentHash = txHashesFile.getData(spentTxNum);
            for (uint16_t i = 0; i < spentTx->outputCount; i++) {
                auto &output = spentTx->getOutput(i);
                if (output.linkedTxNum == txNum) {
                    output.linkedTxNum = 0;
                    blocksci::Inout out = output;
                    out.linkedTxNum = spentTxNum;
                    UTXO utxo(out, input.getType());
                    utxoState.addOutput(utxo, {*spentHash, i});
                }
            }
        }
    }
    
    return state;
}

void rollbackTransactions(size_t blockKeepCount, const ParserConfigurationBase &config) {
    using namespace blocksci;
    
    constexpr auto readwrite = boost::iostreams::mapped_file::readwrite;
    blocksci::FixedSizeFileMapper<blocksci::Block, readwrite> blockFile(config.blockFilePath());
    
    if (blockFile.size() > blockKeepCount) {
        
        auto firstDeletedBlock = blockFile.getData(blockKeepCount);
        auto firstDeletedTxNum = firstDeletedBlock->firstTxIndex;
        
        auto blocksciState = rollbackState(config, blockKeepCount, firstDeletedTxNum);
        
        blocksci::IndexedFileMapper<readwrite, blocksci::RawTransaction>(config.txFilePath()).truncate(firstDeletedTxNum);
        blocksci::FixedSizeFileMapper<blocksci::uint256, readwrite>(config.txHashesFilePath()).truncate(firstDeletedTxNum);
        blocksci::IndexedFileMapper<readwrite, uint32_t>(config.sequenceFilePath()).truncate(firstDeletedTxNum);
        blocksci::ArbitraryFileMapper<readwrite>(config.blockCoinbaseFilePath()).truncate(firstDeletedBlock->coinbaseOffset);
        blockFile.truncate(blockKeepCount);
        
        AddressState(config.addressPath()).rollback(blocksciState);
        AddressWriter(config).rollback(blocksciState);
        FirstSeenIndex(config).rollback(blocksciState);
        AddressDB(config).rollback(blocksciState);
        HashIndexCreator(config).rollback(blocksciState);
    }
}

std::vector<char> HexToBytes(const std::string& hex) {
    std::vector<char> bytes;
    
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(strtol(byteString.c_str(), NULL, 16));
        bytes.push_back(byte);
    }
    
    return bytes;
}

template <typename BlockType>
struct ChainUpdateInfo {
    std::vector<BlockType> blocksToAdd;
    uint32_t splitPoint;
};

template <typename GetBlockHash>
uint32_t findSplitPoint(const blocksci::DataConfiguration &config, uint32_t blockHeight, GetBlockHash getBlockHash) {
    blocksci::ChainAccess oldChain(config, false, 0);
    auto oldBlocks = oldChain.getBlocks();
    
    uint32_t maxSize = std::min(static_cast<uint32_t>(oldBlocks.size()), blockHeight);
    uint32_t splitPoint = static_cast<uint32_t>(maxSize);
    for (uint32_t i = 0; i < maxSize; i++) {
        blocksci::uint256 oldHash = oldBlocks[maxSize - 1 - i].hash;
        blocksci::uint256 newHash = getBlockHash(maxSize - 1 - i);
        if (!(oldHash == newHash)) {
            splitPoint = maxSize - 1 - i;
            break;
        }
    }
    return splitPoint;
}

uint32_t getStartingTxCount(const blocksci::DataConfiguration &config) {
    blocksci::ChainAccess chain(config, false, 0);
    auto blocks = chain.getBlocks();
    if (blocks.size() > 0) {
        auto &lastBlock = chain.getBlocks().back();
        return lastBlock.firstTxIndex + lastBlock.numTxes;
    } else {
        return 0;
    }
}

template <typename ParserTag>
void updateChain(const ParserConfiguration<ParserTag> &config, uint32_t maxBlockNum) {
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

    uint32_t splitPoint = findSplitPoint(config, static_cast<uint32_t>(chainBlocks.size()), [&](uint32_t blockHeight) {
        return chainBlocks[blockHeight].hash;
    });

    std::vector<BlockInfo<ParserTag>> blocksToAdd{chainBlocks.begin() + splitPoint, chainBlocks.end()};
    
    {
        blocksci::ChainAccess chain(config, false, 0);
        auto blocks = chain.getBlocks();
        std::cout << "Starting with chain of " << blocks.size() << " blocks" << std::endl;
        std::cout << "Removing " << blocks.size() - splitPoint << " blocks" << std::endl;
        std::cout << "Adding " << blocksToAdd.size() << " blocks" << std::endl;
    }
    
    std::ios::sync_with_stdio(false);
    
    rollbackTransactions(splitPoint, config);
    
    if (blocksToAdd.size() == 0) {
        return;
    }
    
    uint32_t startingTxCount = getStartingTxCount(config);
    uint32_t maxBlockHeight = static_cast<uint32_t>(blocksToAdd.back().height);
    
    uint32_t totalTxCount = 0;
    for (auto &block : blocksToAdd) {
        totalTxCount += block.nTx;
    }
    
    auto addressDB = ParserIndexCreator::createIndex<AddressDB>(config);
    auto firstSeen = ParserIndexCreator::createIndex<FirstSeenIndex>(config);
    auto hashIndex = ParserIndexCreator::createIndex<HashIndexCreator>(config);
    
    {
        BlockProcessor processor{startingTxCount, totalTxCount, maxBlockHeight};
        UTXOState utxoState{config};
        AddressState addressState{config.addressPath()};
        auto it = blocksToAdd.begin();
        auto end = blocksToAdd.end();
        while (it != end) {
            auto prev = it;
            uint32_t newTxCount = 0;
            while (newTxCount < 1000000 && it != end) {
                newTxCount += it->nTx;
                ++it;
            }
            
            decltype(blocksToAdd) nextBlocks{prev, it};
            
            auto revealedScriptHashes = processor.addNewBlocks(config, nextBlocks, utxoState, addressState);
            
            blocksci::ChainAccess chain{config, false, 0};
            blocksci::ScriptAccess scripts{config};
            
            blocksci::State updateState{chain, scripts};
            
            addressDB.update(revealedScriptHashes, updateState);
            firstSeen.update(revealedScriptHashes, updateState);
            hashIndex.update(revealedScriptHashes, updateState);
        }
    }
    
    {
        blocksci::ChainAccess chain{config, false, 0};
        blocksci::ScriptAccess scripts{config};
        
        blocksci::State state{chain, scripts};
        
        addressDB.complete(state);
        firstSeen.complete(state);
        hashIndex.complete(state);
    }
}

int main(int argc, const char * argv[]) {
    
    namespace po = boost::program_options;
    std::string dataDirectoryString;
    uint32_t maxBlockNum;
    po::options_description general("Options");
    general.add_options()
    ("help", "Print help messages")
    ("output-directory", po::value<std::string>(&dataDirectoryString)->required(), "Path to output parsed data")
    ("max-block", po::value<uint32_t>(&maxBlockNum)->default_value(0), "Max block to scan up to")
    ;

    std::string username;
    std::string password;
    std::string address;
    int port;
    po::options_description rpcOptions("RPC options");
    rpcOptions.add_options()
    ("username", po::value<std::string>(&username), "RPC username")
    ("password", po::value<std::string>(&password), "RPC password")
    ("address", po::value<std::string>(&address)->default_value("127.0.0.1"), "RPC address")
    ("port", po::value<int>(&port)->default_value(9998), "RPC port")
    ;
    
    #ifdef BLOCKSCI_FILE_PARSER
    std::string bitcoinDirectoryString;
    po::options_description fileOptions("File parser options");
    fileOptions.add_options()
    ("coin-directory", po::value<std::string>(&bitcoinDirectoryString), "Path to cryptocurrency directory")
    ;
    #endif
    
    po::options_description all("Allowed options");
    all.add(general);
    #ifdef BLOCKSCI_FILE_PARSER
    all.add(fileOptions);
    #endif
    #ifdef BLOCKSCI_RPC_PARSER
    all.add(rpcOptions);
    #endif
    
    bool validDisk = false;
    bool validRPC = false;
    boost::filesystem::path bitcoinDirectory;
    boost::filesystem::path dataDirectory;
    
    try {
        po::variables_map vm;
        
        po::store(po::parse_command_line(argc, argv, all), vm); // can throw
        
        /** --help option
         */
        if ( vm.count("help")  )
        {
            std::cout << "Basic Command Line Parameter App" << std::endl
            << all << std::endl;
            return 1;
        }
        
        po::notify(vm); // throws on error, so do after help in case
        // there are any problems
        
        dataDirectory = {dataDirectoryString};
        dataDirectory = boost::filesystem::absolute(dataDirectory);
        auto configFile = dataDirectory/"config.ini";
        if (boost::filesystem::exists(configFile)) {
            boost::filesystem::ifstream configStream{configFile};
            po::store(po::parse_config_file(configStream, all), vm);
        }

        bool diskEnabled = false;
        bool rpcEnabled = false;
        
        #ifdef BLOCKSCI_FILE_PARSER
        diskEnabled = true;
        #endif
        
        #ifdef BLOCKSCI_RPC_PARSER
        rpcEnabled = true;
        #endif
        
        if (diskEnabled) {
            if (vm.count("coin-directory")) {
                validDisk = true;
            }
        }
        
        bool enabledSomeRPC = false;
        bool enabledAllRPC = false;
        if (rpcEnabled) {
            enabledSomeRPC = vm.count("username") || vm.count("password");
            enabledAllRPC = vm.count("username") && vm.count("password");
            if (enabledSomeRPC && !enabledAllRPC) {
                throw std::logic_error(std::string("Must specify all RPC username and password"));
            } else if (enabledAllRPC) {
                validRPC = true;
            }
        }
        
        if (rpcEnabled && diskEnabled) {
            if (!validDisk && !validRPC) {
                throw std::logic_error(std::string("Must specify disk or RPC options"));
            }
        } else if (diskEnabled) {
            if (!validDisk) {
                throw std::logic_error(std::string("Must specify disk options"));
            }
        } else if (rpcEnabled) {
            if (!validRPC) {
                throw std::logic_error(std::string("Must specify RPC options"));
            }
        }
        
        boost::property_tree::ptree rootPTree;
        
        if (validDisk) {
            bitcoinDirectory = {bitcoinDirectoryString};
            bitcoinDirectory = boost::filesystem::absolute(bitcoinDirectory);
            rootPTree.put("coin-directory", bitcoinDirectory.native());
        }
        
        if (validRPC) {
            rootPTree.put("username", username);
            rootPTree.put("password", password);
            rootPTree.put("address", address);
            rootPTree.put("port", port);
        }
        
        if(!(boost::filesystem::exists(dataDirectory))){
            boost::filesystem::create_directory(dataDirectory);
        }
        
        boost::filesystem::ofstream configStream{configFile};
        boost::property_tree::write_ini(configStream, rootPTree);
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << all << std::endl;
        return -1;
    }
    
    
    if (validDisk) {
        #ifdef BLOCKSCI_FILE_PARSER
        ParserConfiguration<FileTag> config{bitcoinDirectory, dataDirectory};
//        if (bitcoinDirectoryString.find("bitcoin") != std::string::npos || bitcoinDirectoryString.find("Bitcoin") != std::string::npos) {
//            if (maxBlockNum > 478559 || maxBlockNum == 0) {
//                maxBlockNum = 478559;
//            }
//        }
        // replayBlock(config, 177618);
        updateChain(config, maxBlockNum);;
//        updateFirstSeenIndex(config, 0);
        #endif
    } else if (validRPC) {
        #ifdef BLOCKSCI_RPC_PARSER
        ParserConfiguration<RPCTag> config(username, password, address, port, dataDirectory);
        updateChain(config, maxBlockNum);
        #endif
    }
    
    
    return 0;
}
