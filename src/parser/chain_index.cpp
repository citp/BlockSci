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

#include <blocksci/hash.hpp>

#include <leveldb/db.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <sstream>
#include <fstream>
#include <iostream>

#ifdef BLOCKSCI_FILE_PARSER

BlockInfo::BlockInfo(const CBlockHeader &h, int fileNum, unsigned int dataPos, uint32_t length_, unsigned int numTxes, const FileParserConfiguration &config) : height(-1), nFile(fileNum), nDataPos(dataPos), length(length_), nTx(numTxes), header(h) {
    hash = config.workHashFunction(reinterpret_cast<const char *>(&h), sizeof(CBlockHeader));
}

ChainIndex::ChainIndex(const FileParserConfiguration &config_) : config(config_) {
    boost::filesystem::ifstream file(config.blockListPath(), std::ios::binary);
    if (file.good()) {
        uint64_t length;
        file.read(reinterpret_cast<char *>(&length), sizeof(length));
        blockList.resize(length);
        file.read(reinterpret_cast<char *>(blockList.data()), static_cast<long>(sizeof(BlockInfo) * length));
    }
    updateFromFilesystem();
}

ChainIndex::~ChainIndex() {
    boost::filesystem::ofstream file(config.blockListPath(), std::ios::binary);
    uint64_t length = blockList.size();
    file.write(reinterpret_cast<char *>(&length), sizeof(length));
    file.write(reinterpret_cast<char *>(blockList.data()), static_cast<long>(sizeof(BlockInfo) * length));
}

void ChainIndex::updateFromFilesystem() {
    int fileNum = 0;
    unsigned int filePos = 0;

    if (!blockList.empty()) {
        auto &lastBlock = blockList.back();
        fileNum = lastBlock.nFile;
        filePos = lastBlock.nDataPos + lastBlock.length;
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
                blockList.emplace_back(header, fileNum, blockStartOffset, length, numTxes, config);
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

int ChainIndex::updateHeight(size_t blockNum, const std::unordered_map<blocksci::uint256, size_t> &indexMap) {
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

std::vector<BlockInfo> ChainIndex::generateChain(uint32_t maxBlockHeight) const {
    
    std::unordered_map<blocksci::uint256, size_t> indexMap;
    indexMap.reserve(blockList.size());
    size_t blockNum = 0;
    for (const auto &blockInfo : blockList) {
        indexMap[blockInfo.hash] = blockNum;
        blockNum++;
    }
    
    uint32_t curMax = maxBlockHeight == 0 ? std::numeric_limits<int>::max() : maxBlockHeight;
    
    std::vector<BlockInfo> chain;
    while (true) {
        chain.clear();
        const BlockInfo *maxHeightBlock = nullptr;
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
        
        const BlockInfo *block = maxHeightBlock;
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

#endif
