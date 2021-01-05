//
//  chain_index.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "chain_index.hpp"
#include "parser_configuration.hpp"
#include "safe_mem_reader.hpp"
#include "preproccessed_block.hpp"

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <cereal/archives/binary.hpp>

#include <cmath>
#include <future>
#include <iostream>

#ifdef BLOCKSCI_FILE_PARSER

BlockInfoBase::BlockInfoBase(const blocksci::uint256 &hash_, const CBlockHeader &h, uint32_t size_, unsigned int numTxes, uint32_t inputCount_, uint32_t outputCount_) : hash(hash_), header(h), height(-1), size(size_), nTx(numTxes), inputCount(inputCount_), outputCount(outputCount_) {}

BlockInfo<FileTag>::BlockInfo(const CBlockHeader &h, uint32_t size_, unsigned int numTxes, uint32_t inputCount_, uint32_t outputCount_, const ChainDiskConfiguration &config, int fileNum, unsigned int dataPos) : BlockInfoBase(config.workHashFunction(reinterpret_cast<const char *>(&h), sizeof(CBlockHeader)), h, size_, numTxes, inputCount_, outputCount_), nFile(fileNum), nDataPos(dataPos) {}

// The 0 should be replaced with info.bits converted to string
BlockInfo<RPCTag>::BlockInfo(const blockinfo_t &info, blocksci::BlockHeight height_) : 
BlockInfoBase(
    blocksci::uint256S(info.hash), 
    {info.version, blocksci::uint256S(info.previousblockhash), blocksci::uint256S(info.merkleroot), info.time, 0, info.nonce}, 
    static_cast<uint32_t>(info.size), 
    static_cast<uint32_t>(info.tx.size()), 0, 0
    ), tx(info.tx) {
    height = height_;
}

int maxBlockFileNum(int startFile, const ParserConfiguration<FileTag> &config) {
    int fileNum = startFile;
    while (config.pathForBlockFile(fileNum).exists()) {
        fileNum++;
    }
    return fileNum - 1;
}

namespace {
    std::vector<BlockInfo<FileTag>> readBlocksImpl(SafeMemReader &reader, int fileNum, const ChainDiskConfiguration &config) {
        try {
            std::vector<BlockInfo<FileTag>> blocks;
            // Read blocks in loop while we can...
            while (reader.has(sizeof(uint32_t))) {
                auto magic = reader.readNext<uint32_t>();
                if (magic != config.blockMagic) {
                    break;
                }
                auto length = reader.readNext<uint32_t>();
                auto blockStartOffset = reader.offset();
                assert(reader.has(length));
                while (reader.peakNext<uint32_t>() == config.blockMagic) {
                    // The previous block must have been cut off
                    // See https://github.com/bitcoin/bitcoin/issues/8614
                    reader.advance(sizeof(uint32_t));
                    length = reader.readNext<uint32_t>();
                    blockStartOffset = reader.offset();
                }
                auto header = reader.readNext<CBlockHeader>();
                auto numTxes = reader.readVariableLengthInteger();
                uint32_t inputCount = 0;
                uint32_t outputCount = 0;
                for (size_t i = 0; i < numTxes; i++) {
                    TransactionHeader h(reader);
                    inputCount += h.inputCount;
                    outputCount += h.outputCount;
                }
                // The next two lines bring the reader to the end of this block
                reader.reset(blockStartOffset);
                reader.advance(length);
                inputCount--;
                blocks.emplace_back(header, length, numTxes, inputCount, outputCount, config, fileNum, blockStartOffset);
            }
            return blocks;
        } catch (const std::out_of_range &e) {
            std::cerr << "Failed to read block header information"
            << " from " << reader.getPath()
            << " at offset " << reader.offset()
            << ": " << e.what() << "\n";
            throw;
        }
    }
}

std::vector<BlockInfo<FileTag>> readBlocksInfo(int fileNum, const ParserConfiguration<FileTag> &config) {
    auto blockFilePath = config.pathForBlockFile(fileNum);
    SafeMemReader reader{blockFilePath.str()};
    return readBlocksImpl(reader, fileNum, config.diskConfig);
}

template <>
void ChainIndex<FileTag>::update(const ConfigType &config, blocksci::BlockHeight /*maxblockHeight*/) {
    int fileNum = 0;
    unsigned int filePos = 0;

    if (!blockList.empty()) {
        fileNum = newestBlock.nFile;
        filePos = newestBlock.nDataPos + newestBlock.size;
    }

    auto firstFile = fileNum;
    
    auto maxFileNum = maxBlockFileNum(fileNum, config);
    
    auto localConfig = config;
    
    std::mutex m;
    
    std::cout.setf(std::ios::fixed,std::ios::floatfield);
    std::cout.precision(1);
    auto fileCount = maxFileNum - fileNum + 1;
    int filesDone = 0;
    using namespace std::chrono_literals;
    std::atomic<int> activeThreads{0};
    {
        std::vector<std::future<void>> blockFutures;
        for (; fileNum <= maxFileNum; fileNum++) {
            while (activeThreads > 20) {
                std::this_thread::sleep_for(500ms);
            }
            blockFutures.emplace_back(std::async(std::launch::async, [&](int fileNum) {
                activeThreads++;
                // Determine block file path
                auto blockFilePath = localConfig.pathForBlockFile(fileNum);
                SafeMemReader reader{blockFilePath.str()};
                // Logic for resume from last processed block, note blockStartOffset and length below
                if (fileNum == firstFile) {
                    reader.reset(filePos);
                }
                auto blocks = readBlocksImpl(reader, fileNum, localConfig.diskConfig);
                
                if (fileNum == maxFileNum && blocks.size() > 0) {
                    newestBlock = blocks.back();
                }
                activeThreads--;
                std::lock_guard<std::mutex> lock(m);
                
                for (auto &block : blocks) {
                    blockList[block.hash] = block;
                }
                
                filesDone++;
                std::cout << "\r" << (static_cast<double>(filesDone) / static_cast<double>(fileCount)) * 100 << "% done fetching block headers" << std::flush;
                
            }, fileNum));
        }
    }
    
    std::cout << std::endl;
    
    std::unordered_multimap<blocksci::uint256, blocksci::uint256> forwardHashes;

    // Fill forwardHashes with pairs of ((prevBlockHash) -> (currentBlockHash)) for every block
    for (auto &pair : blockList) {
        forwardHashes.emplace(pair.second.header.hashPrevBlock, pair.second.hash);
    }
    
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    
    std::vector<std::pair<blocksci::uint256, blocksci::BlockHeight>> queue;
    
    queue.emplace_back(nullHash, 0);
    while (!queue.empty()) {
        blocksci::uint256 blockHash;
        blocksci::BlockHeight height;
        std::tie(blockHash, height) = queue.back();
        queue.pop_back();
        for (auto ret = forwardHashes.equal_range(blockHash); ret.first != ret.second; ++ret.first) {
            auto &block = blockList.at(ret.first->second);
            block.height = height + 1;
            queue.emplace_back(block.hash, block.height);
        }
    }
    
}

template<>
void ChainIndex<RPCTag>::update(const ConfigType &config, blocksci::BlockHeight blockHeight) {
    try {
        BitcoinAPI bapi{config.createBitcoinAPI()};
        
        
        auto maxBlockHeight = static_cast<blocksci::BlockHeight>(bapi.getblockcount());
        if (blockHeight <= 0) {
            blockHeight = maxBlockHeight + blockHeight;
        }
        if (blockHeight > maxBlockHeight) {
            blockHeight = maxBlockHeight;
        }
        
        auto splitPoint = findSplitPointIndex(blockHeight, [&](blocksci::BlockHeight h) {
            return blocksci::uint256S(bapi.getblockhash(static_cast<int>(h)));
        });
        
        std::cout.setf(std::ios::fixed,std::ios::floatfield);
        std::cout.precision(1);
        blocksci::BlockHeight numBlocks = blockHeight - splitPoint;
        auto percentage = static_cast<double>(static_cast<int>(numBlocks)) / 1000.0;
        auto percentageMarker = static_cast<blocksci::BlockHeight>(std::ceil(percentage));
        
        for (blocksci::BlockHeight i = splitPoint; i < blockHeight; i++) {
            std::string blockhash = bapi.getblockhash(static_cast<int>(i));
            BlockType block{bapi.getblock(blockhash), i};
            blockList.emplace(block.hash, block);
            auto count = i - splitPoint;
            if (count % percentageMarker == 0) {
                std::cout << "\r" << (static_cast<double>(static_cast<int>(count)) / static_cast<double>(static_cast<int>(numBlocks))) * 100 << "% done fetching block headers" << std::flush;
            }
            if (i == blockHeight - 1) {
                newestBlock = block;
            }
        }
        
        std::cout << std::endl;
    } catch (const BitcoinException &e) {
        std::cout << std::endl;
        std::cerr << "Error while interacting with RPC: " << e.what() << std::endl;
        throw;
    }
    std::cout << std::endl;
}

#endif
