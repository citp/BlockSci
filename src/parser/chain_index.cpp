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

#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/hash.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <leveldb/db.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>

#include <sstream>
#include <fstream>
#include <iostream>

#ifdef BLOCKSCI_FILE_PARSER


BlockInfoBase::BlockInfoBase(const blocksci::uint256 &hash_, const CBlockHeader &h, uint32_t size_, unsigned int numTxes) : hash(hash_), header(h), height(-1), size(size_), nTx(numTxes) {}

BlockInfo<FileTag>::BlockInfo(const CBlockHeader &h, uint32_t size_, unsigned int numTxes, const ParserConfiguration<FileTag> &config, int fileNum, unsigned int dataPos) : BlockInfoBase(config.workHashFunction(reinterpret_cast<const char *>(&h), sizeof(CBlockHeader)), h, size_, numTxes), nFile(fileNum), nDataPos(dataPos) {}

// The 0 should be replaced with info.bits converted to string
BlockInfo<RPCTag>::BlockInfo(const blockinfo_t &info, uint32_t height_) : BlockInfoBase(blocksci::uint256S(info.hash), {info.version, blocksci::uint256S(info.previousblockhash), blocksci::uint256S(info.merkleroot), info.time, 0, info.nonce}, info.size, info.tx.size()), tx(info.tx) {
    height = static_cast<int>(height_);
}

template <>
void ChainIndex<FileTag>::update() {
    int fileNum = 0;
    unsigned int filePos = 0;

    if (!blockList.empty()) {
        auto &lastBlock = blockList.back();
        fileNum = lastBlock.nFile;
        filePos = lastBlock.nDataPos + lastBlock.size;
    }

    auto firstFile = fileNum;

    while (true) {
        // determine block file path
        auto blockFilePath = config.pathForBlockFile(fileNum);
        if (!boost::filesystem::exists(blockFilePath)) {
            std::cout << "No block file " << blockFilePath << "\n";
            break;
        }

        SafeMemReader reader{blockFilePath};
        
        std::cout << "Reading " << blockFilePath << "...\n";
        
        try {
            // logic for resume from last processed block, note blockStartOffset and length below
            if (fileNum == firstFile && filePos > 0) {
                reader.reset(filePos);
            }
            
            // read blocks in loop while we can...
            while (reader.has(sizeof(uint32_t))) {
                auto magic = reader.readNext<uint32_t>();
                if (magic != config.blockMagic) {
                    break;
                }
                auto length = reader.readNext<uint32_t>();
                auto blockStartOffset = reader.offset();
                auto header = reader.readNext<CBlockHeader>();
                auto numTxes = reader.readVariableLengthInteger();
                // The next two lines bring the reader to the end of this block
                reader.reset(blockStartOffset);
                reader.advance(length);
                blockList.emplace_back(header, length, numTxes, config, fileNum, blockStartOffset);
            }
        } catch (const std::out_of_range &e) {
            std::cerr << "Failed to read block header information"
            << " from " << blockFilePath
            << " at offset " << reader.offset()
            << ".\n";
            throw;
        }
        
        // move to next block file
        fileNum++;
        filePos = 0;
    }
    
    std::unordered_map<blocksci::uint256, size_t> indexMap;
    indexMap.reserve(blockList.size());
    size_t blockNum = 0;
    for (const auto &blockInfo : blockList) {
        indexMap[blockInfo.hash] = blockNum;
        blockNum++;
    }

    for (auto &i : blockList) {
        i.height = -1;
    }
    
    for (size_t i = 0; i < blockList.size(); i++) {
        updateHeight(i, indexMap);
    }
}

template<>
void ChainIndex<RPCTag>::update() {
    try {
        BitcoinAPI bapi{config.createBitcoinAPI()};
        auto blockHeight = static_cast<uint32_t>(bapi.getblockcount());

        
        uint32_t splitPoint = findSplitPointIndex(blockHeight, [&](uint32_t h) {
            return blocksci::uint256S(bapi.getblockhash(static_cast<int>(h)));
        });
        
        std::cout.setf(std::ios::fixed,std::ios::floatfield);
        std::cout.precision(1);
        uint32_t numBlocks = blockHeight - splitPoint;
        auto percentage = static_cast<double>(numBlocks) / 1000.0;
        uint32_t percentageMarker = static_cast<uint32_t>(std::ceil(percentage));
        
        for (uint32_t i = splitPoint; i < blockHeight; i++) {
            std::string blockhash = bapi.getblockhash(static_cast<int>(i));
            blockList.emplace_back(bapi.getblock(blockhash), i);
            uint32_t count = i - splitPoint;
            if (count % percentageMarker == 0) {
                std::cout << "\r" << (static_cast<double>(count) / static_cast<double>(numBlocks)) * 100 << "% done fetching block headers" << std::flush;
            }
        }
        std::cout << std::endl;
        blockList = generateChain(0);
    } catch (const BitcoinException &e) {
        std::cout << std::endl;
        std::cerr << "Error while interacting with RPC: " << e.what() << std::endl;
        throw;
    }
}

template<typename ParseTag>
int ChainIndex<ParseTag>::updateHeight(size_t blockNum, const std::unordered_map<blocksci::uint256, size_t> &indexMap) {
    auto &block = blockList[blockNum];
    if (block.height == -1) {
        if (block.header.hashPrevBlock.IsNull()) {
            block.height = 0;
        } else {
            auto it = indexMap.find(block.header.hashPrevBlock);
            if(it != indexMap.end()) {
                block.height = updateHeight(it->second, indexMap) + 1;
            } else {
                block.height = -1;
            }
            
        }
    }
    return block.height;
}

template<typename ParseTag>
std::vector<typename ChainIndex<ParseTag>::BlockType> ChainIndex<ParseTag>::generateChain(uint32_t maxBlockHeight) const {
    
    std::unordered_map<blocksci::uint256, size_t> indexMap;
    indexMap.reserve(blockList.size());
    size_t blockNum = 0;
    for (const auto &blockInfo : blockList) {
        indexMap[blockInfo.hash] = blockNum;
        blockNum++;
    }
    
    uint32_t curMax = maxBlockHeight == 0 ? std::numeric_limits<int>::max() : maxBlockHeight;
    
    std::vector<BlockType> chain;
    while (true) {
        chain.clear();
        const BlockType *maxHeightBlock = nullptr;
        uint32_t maxHeight = 0;
        for (auto &possibleMaxBlock : blockList) {
            if (static_cast<uint32_t>(possibleMaxBlock.height) > maxHeight && static_cast<uint32_t>(possibleMaxBlock.height) <= curMax) {
                maxHeightBlock = &possibleMaxBlock;
                maxHeight = static_cast<uint32_t>(possibleMaxBlock.height);
            }
        }
        
        if (maxHeightBlock == nullptr) {
            return chain;
        }
        
        auto block = maxHeightBlock;
        while (true) {
            if (static_cast<uint32_t>(block->height) < curMax) {
                chain.push_back(*block);
            }
            if (block->height > 0) {
                auto it = indexMap.find(block->header.hashPrevBlock);
                if (it == indexMap.end()) {
                    curMax = static_cast<uint32_t>(block->height) - 2;
                    break;
                } else {
                    block = &blockList[it->second];
                }
            } else {
                std::reverse(chain.begin(), chain.end());
                return chain;
            }
        }
    }
}

template std::vector<typename ChainIndex<FileTag>::BlockType> ChainIndex<FileTag>::generateChain(uint32_t maxBlockHeight) const;
template std::vector<typename ChainIndex<RPCTag>::BlockType> ChainIndex<RPCTag>::generateChain(uint32_t maxBlockHeight) const;

#endif
