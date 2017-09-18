//
//  main.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 12/30/15.
//  Copyright © 2015 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "config.hpp"
#include "utilities.hpp"
#include "parser_configuration.hpp"
#include "utxo_state.hpp"
#include "address_state.hpp"
#include "chain_index.hpp"
#include "preproccessed_block.hpp"
#include "block_processor.hpp"
#include "address_db.hpp"
#include "first_seen_index.hpp"
#include "hash_index.hpp"
#include "block_replayer.hpp"
#include "address_writer.hpp"

#include <blocksci/data_access.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/chain/block.hpp>

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
#include <stdio.h>
#include <assert.h>

void rollbackTransactions(size_t blockKeepCount, const ParserConfiguration &config) {
    using namespace blocksci;
    
    blocksci::ChainAccess chain(config, false, 0);
    auto blocks = chain.getBlocks();
    
    if (blocks.size() > blockKeepCount) {
        UTXOState utxoState{config};
        blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> txFile{config.txFilePath()};
        blocksci::FixedSizeFileMapper<blocksci::uint256, boost::iostreams::mapped_file::readwrite> txHashesFile{config.txHashesFilePath()};
        blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, uint32_t> sequenceFile{config.sequenceFilePath()};
        
        std::unordered_map<ScriptType::Enum, uint32_t> addressCounts;
        ScriptAccess scripts{config};
        ScriptFirstSeenAccess firstSeenIndex(config);
        
        auto &firstDeletedBlock = blocks[blockKeepCount];
        auto firstDeletedTxNum = firstDeletedBlock.firstTxIndex;
        
        auto totalTxCount = chain.txCount();
        
        for (uint32_t txNum = totalTxCount - 1; txNum >= firstDeletedTxNum; txNum--) {
            auto tx = Transaction::txWithIndex(chain, txNum);
            auto hash = tx.getHash(chain);
            
            for (uint16_t i = 0; i < tx.outputCount(); i++) {
                auto &output = tx.outputs()[i];
                if (output.getAddress().getFirstTransactionIndex(firstSeenIndex) == txNum) {
                    auto &prevValue = addressCounts[scriptType(output.getType())];
                    auto addressNum = output.getAddress().addressNum;
                    if (addressNum < prevValue) {
                        prevValue = addressNum;
                    }
                }
                if (output.getType() != AddressType::Enum::NULL_DATA) {
                    utxoState.spendOutput({hash, i});
                }
            }
            
            for(auto &input : tx.inputs()) {
                auto spentTx = input.getSpentTx(chain);
                auto txPointer = txFile.getData(spentTx.txNum);
                for (uint16_t i = 0; i < spentTx.outputCount(); i++) {
                    auto &output = txPointer->getOutput(i);
                    if (output.linkedTxNum == tx.txNum) {
                        output.linkedTxNum = 0;
                        Inout out = output;
                        out.linkedTxNum = spentTx.txNum;
                        UTXO utxo(out, input.getType());
                        utxoState.addOutput(utxo, {spentTx.getHash(chain), i});
                    }
                }
            }
            
            
            AddressState addressState{config};
            addressState.removeAddresses(addressCounts);
            
            txFile.truncate(firstDeletedTxNum);
            txHashesFile.truncate(firstDeletedTxNum);
            sequenceFile.truncate(firstDeletedTxNum);
            
            blocksci::ArbitraryFileMapper<boost::iostreams::mapped_file::readwrite> blockCoinbaseFile(config.blockCoinbaseFilePath());
            blocksci::FixedSizeFileMapper<blocksci::Block, boost::iostreams::mapped_file::readwrite> blockFile(config.blockFilePath());
            
            blockCoinbaseFile.truncate(firstDeletedBlock.coinbaseOffset);
            blockFile.truncate(blockKeepCount);
            
            AddressWriter addressWriter{config};
            for (auto pair : addressCounts) {
                addressWriter.truncate(pair.first, pair.second);
            }
        }
    }
}

std::vector<char> HexToBytes(const std::string& hex) {
    std::vector<char> bytes;
    
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    
    return bytes;
}

/*
 std::string hexString("0100000001dce0625ea257b0f0e772d61050003020bad0dbc42c96a51491d8c8e23698d2d7010000006a473044022027bce535775f7eceec7a6dacef9ae84e3c5279ab83158e5a5472ded772b8206c02207f109a5f01114938f11e4fef3949e6930d2df50d115e6d577a8e07d3a56e14b20121032a0701b1f33f3b38fd23f23633c69a80b1c878b372b342e6bcbe963e2f88f7f5ffffffff01df280300000000001976a914becca086d70b43ee656d48ab038f09ce5ce0b95588ac00000000");
 */

#ifdef BLOCKSCI_FILE_PARSER
void printTxInfo(std::string &hexString) {
    auto bytes = HexToBytes(hexString);
    const char *bytesPtr = &bytes[0];
    RawTransaction tx;
    tx.load(&bytesPtr, 0, true);
    
    std::cout << "Size bytes: " << tx.sizeBytes << std::endl;
    std::cout << "Locktime: " << tx.locktime << std::endl;
    std::cout << "Version: " << tx.version << std::endl;
    for (auto &input : tx.inputs) {
        std::cout << "Spending " << input.rawOutputPointer << " sequence : " << input.sequenceNum << std::endl;
    }
    
    for (auto &output : tx.outputs) {
        std::cout << "Sent " << output.value << "\n";
    }
}
#endif

template <typename GetBlockHash>
uint32_t findSplitPoint(const ParserConfiguration &config, uint32_t blockHeight, GetBlockHash getBlockHash) {
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

template <typename BlockType>
struct ChainUpdateInfo {
    std::vector<BlockType> blocksToAdd;
    uint32_t splitPoint;
};


#ifdef BLOCKSCI_FILE_PARSER

uint32_t txCount(const BlockInfo &block) {
    return block.nTx;
}

ChainUpdateInfo<BlockInfo> prepareChain(const FileParserConfiguration &config, uint32_t maxBlockNum) {
    ChainIndex index(config);
    
    auto chain = index.generateChain(maxBlockNum);
    
    uint32_t splitPoint = findSplitPoint(config, chain.size(), [&](uint32_t blockHeight) {
        return chain[blockHeight].hash;
    });
    
    return {{chain.begin() + splitPoint, chain.end()}, splitPoint};
}

#endif

#ifdef BLOCKSCI_RPC_PARSER

uint32_t txCount(const blockinfo_t &block) {
    return block.tx.size();
}

ChainUpdateInfo<blockinfo_t> prepareChain(const RPCParserConfiguration &config, uint32_t maxBlockNum) {
    BitcoinAPI bapi{config.createBitcoinAPI()};
    if (maxBlockNum == 0) {
        maxBlockNum = std::numeric_limits<uint32_t>::max();
    }
    auto blockHeight = static_cast<size_t>(std::min(maxBlockNum, static_cast<uint32_t>(bapi.getblockcount())));
    
    uint32_t splitPoint = findSplitPoint(config, blockHeight, [&](uint32_t blockHeight) {
        return blocksci::uint256S(bapi.getblockhash(blockHeight));
    });
    
    std::cout.setf(std::ios::fixed,std::ios::floatfield);
    std::cout.precision(1);
    uint32_t numBlocks = blockHeight - splitPoint;
    auto percentage = static_cast<double>(numBlocks) / 1000.0;
    uint32_t percentageMarker = static_cast<uint32_t>(std::ceil(percentage));
    
    std::vector<blockinfo_t> blocksToAdd;
    blocksToAdd.reserve(numBlocks);
    for (uint32_t i = splitPoint; i < blockHeight; i++) {
        std::string blockhash = bapi.getblockhash(i);
        blocksToAdd.push_back(bapi.getblock(blockhash));
        int count = i - splitPoint;
        if (count % percentageMarker == 0) {
            std::cout << "\r" << (static_cast<double>(count) / static_cast<double>(numBlocks)) * 100 << "% done fetching block headers" << std::flush;
        }
        
        
    }
    std::cout << std::endl;

    return {blocksToAdd, splitPoint};
}

#endif


uint32_t getStartingTxCount(const ParserConfiguration &config) {
    blocksci::ChainAccess chain(config, false, 0);
    auto blocks = chain.getBlocks();
    if (blocks.size() > 0) {
        auto &lastBlock = chain.getBlocks().back();
        return lastBlock.firstTxIndex + lastBlock.numTxes;
    } else {
        return 0;
    }
}
template <typename BlockType>
void printUpdateInfo(const ParserConfiguration &config, const ChainUpdateInfo<BlockType> &chainUpdateInfo) {
    blocksci::ChainAccess chain(config, false, 0);
    auto blocks = chain.getBlocks();
    std::cout << "Starting with chain of " << blocks.size() << " blocks" << std::endl;
    std::cout << "Removing " << blocks.size() - chainUpdateInfo.splitPoint << " blocks" << std::endl;
    std::cout << "Adding " << chainUpdateInfo.blocksToAdd.size() << " blocks" << std::endl;
}

template <typename ConfigType, typename BlockType>
std::vector<uint32_t> addNewBlocks(const ConfigType &config, const std::vector<BlockType> &nextBlocks, uint32_t startingTxCount, uint32_t currentCount, uint32_t totalTxCount, uint32_t maxBlockHeight, UTXOState &utxoState, AddressState &addressState) {
    BlockProcessor processor;
    
    auto importer = std::async(std::launch::async, [&] {
        processor.readNewBlocks(config, nextBlocks, startingTxCount);
    });
    
    auto hashCalculator = std::async(std::launch::async, [&] {
        processor.calculateHashes(config);
    });
    
    auto utxoProcessor = std::async(std::launch::async, [&] {
        processor.processUTXOs(config, utxoState);
    });
    
    auto addressProcessor = std::async(std::launch::async, [&] {
        return processor.processAddresses(config, addressState, currentCount, totalTxCount, maxBlockHeight);
    });
    
    importer.get();
    hashCalculator.get();
    utxoProcessor.get();
    return addressProcessor.get();
}

template <typename ConfigType>
void updateChain(const ConfigType &config, uint32_t maxBlockNum) {
    using namespace std::chrono_literals;
    
    auto chainUpdateInfo = prepareChain(config, maxBlockNum);
    
    printUpdateInfo(config, chainUpdateInfo);
    
    std::ios::sync_with_stdio(false);
    
    rollbackTransactions(chainUpdateInfo.splitPoint, config);
    
    auto blocksToAdd = std::move(chainUpdateInfo.blocksToAdd);
    
    AddressDB addressDB{config};
    FirstSeenIndex firstSeen{config};
    HashIndex hashIndex{config};
    
    uint32_t startingTxCount = getStartingTxCount(config);
    uint32_t blockNum = 0;
    uint32_t lastBlockNum = 0;
    uint32_t maxBlockHeight = blocksToAdd.back().height;
    
    UTXOState utxoState{config};
    AddressState addressState{config};
    
    uint32_t totalTxCount = 0;
    for (auto &block : blocksToAdd) {
        totalTxCount += txCount(block);
    }
    
    uint32_t currentCount = 0;
    
    while (blockNum < blocksToAdd.size()) {
        
        uint32_t newTxCount = 0;
        
        for (; blockNum < blocksToAdd.size(); blockNum++) {
            auto &block = blocksToAdd[blockNum];
            newTxCount += txCount(block);
            
            if (newTxCount > 1'000'000) {
                break;
            }
        }
        
        decltype(blocksToAdd) nextBlocks{blocksToAdd.begin() + lastBlockNum, blocksToAdd.begin() + blockNum};
        
        auto revealedScriptHashes = addNewBlocks(config, nextBlocks, startingTxCount, currentCount, totalTxCount, maxBlockHeight, utxoState, addressState);
        
        currentCount += newTxCount;
        startingTxCount += newTxCount;
        lastBlockNum = blockNum;
        
        addressDB.update(revealedScriptHashes);
        firstSeen.update(revealedScriptHashes);
        hashIndex.update(revealedScriptHashes);
    }
    
    addressDB.complete();
    firstSeen.complete();
    hashIndex.complete();
    
    addressDB.preDestroy();
    firstSeen.preDestroy();
    hashIndex.preDestroy();
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
        boost::filesystem::path bitcoinDirectory{bitcoinDirectoryString};
        bitcoinDirectory = boost::filesystem::absolute(bitcoinDirectory);
        FileParserConfiguration config{bitcoinDirectory, dataDirectory};
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
        RPCParserConfiguration config(username, password, address, port, dataDirectory);
        updateChain(config, maxBlockNum);
        #endif
    }
    
    
    return 0;
}
