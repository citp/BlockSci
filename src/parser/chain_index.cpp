//
//  chain_index.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "chain_index.hpp"
#include "utilities.hpp"
#include "parser_configuration.hpp"

#include <blocksci/hash.hpp>

#include <leveldb/db.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <sstream>
#include <fstream>
#include <iostream>

#ifdef BLOCKSCI_FILE_PARSER

struct CBlockHeader
{
    // header
    int32_t nVersion;
    blocksci::uint256 hashPrevBlock;
    blocksci::uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
};

BlockInfo::BlockInfo(const CBlockHeader &h, int fileNum, unsigned int dataPos, unsigned int numTxes, const FileParserConfiguration &config) : prevHash(h.hashPrevBlock), height(-1), nFile(fileNum), nDataPos(dataPos), nTx(numTxes), nVersion(h.nVersion), hashMerkleRoot(h.hashMerkleRoot), nTime(h.nTime), nBits(h.nBits), nNonce(h.nNonce) {
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
    size_t filePos = 0;
    
    if (!blockList.empty()) {
        auto &lastBlock = blockList.back();
        fileNum = lastBlock.nFile;
        filePos = lastBlock.nDataPos;
    }
    
    int firstFile = fileNum;
    
    while (true) {
        auto blockPath = config.pathForBlockFile(fileNum);
        if (!boost::filesystem::exists(blockPath)) {
            std::cout << "No block file " << blockPath << "\n";
            break;
        }
        
        boost::iostreams::mapped_file_source fileMap;
        fileMap.open(blockPath);
        
        const char *startPos = fileMap.data() + filePos;
        assert(filePos <= fileMap.size());
        if (filePos + 4 > fileMap.size()) {
            continue;
        }
        if (fileNum == firstFile && filePos > 0) {
            startPos -= sizeof(uint32_t);
            uint32_t length = readNext<uint32_t>(&startPos);
            startPos += length;
        }
        // config.blockMagic
        // 0xdab5bffa
        // 0x0709110b
        uint32_t magic = 0;
        while ((magic = readNext<uint32_t>(&startPos)) == config.blockMagic) {
            uint32_t length = readNext<uint32_t>(&startPos);
            filePos = static_cast<size_t>(startPos - fileMap.data());
            auto header = reinterpret_cast<const CBlockHeader *>(startPos);
            size_t blockHeaderSize = 80;
            const char *buffer = startPos + blockHeaderSize;
            uint32_t numTxes = readVariableLengthInteger(&buffer);
            startPos += length;
            blockList.push_back(BlockInfo(*header, fileNum, static_cast<uint32_t>(filePos), numTxes, config));
        }
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

    for (size_t i = 0; i < blockList.size(); i++) {
        blockList[i].height = -1;
    }
    
    for (size_t i = 0; i < blockList.size(); i++) {
        updateHeight(i, indexMap);
    }
}

int ChainIndex::updateHeight(size_t blockNum, const std::unordered_map<blocksci::uint256, size_t> &indexMap) {
    auto &block = blockList[blockNum];
    if (block.height == -1) {
        if (block.prevHash.IsNull()) {
            block.height = 0;
        } else {
            auto it = indexMap.find(block.prevHash);
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
                auto it = indexMap.find(block->prevHash);
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
