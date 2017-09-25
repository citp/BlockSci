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

class SafeMemReader {
public:
    typedef boost::iostreams::mapped_file_source::size_type size_type;

    explicit SafeMemReader(boost::iostreams::mapped_file_source &fileMap) {
        begin = fileMap.begin();
        end = fileMap.end();
        pos = begin;
    }

    bool has(size_type n) {
        return pos + n <= end;
    }

    template<typename Type>
    bool readNext(Type* val) {
        auto size = sizeof(Type);
        if (!has(size)) {
            return false;
        }
        memcpy(val, pos, size);
        pos += size;
        return true;
    }

    // reads a variable length integer.
    // See the documentation from here:  https://en.bitcoin.it/wiki/Protocol_specification#Variable_length_integer
    bool readVariableLengthInteger(uint32_t* val) {
        uint8_t v;
        if (!readNext(&v)) {
            return false;
        };

        if ( v < 0xFD ) { // If it's less than 0xFD use this value as the unsigned integer
            *val = static_cast<uint32_t>(v);
            return true;
        } else if (v == 0xFD) {
            uint16_t part;
            if (!readNext(&part)) {
                return false;
            };
            *val = static_cast<uint32_t>(part);
            return true;
        } else if (v == 0xFE) {
            return readNext(&val);
        } else {
            uint64_t part;
            if (!readNext(&part)) {
                return false;
            };
            *val = static_cast<uint32_t>(part); // TODO: maybe we should not support this here, we lose data
            return true;
        }
    }

    bool advance(size_type n) {
        if (!has(n)) {
            return false;
        }
        pos += n;
        return true;
    }

    bool rewind(size_type n) {
        if (pos < begin + n) {
            return false;
        }
        pos -= n;
        return true;
    }

    bool reset() {
        pos = begin;
        return true;
    }

    bool reset(size_type n) {
        if (begin + n > end) {
            return false;
        }
        pos = begin + n;
        return true;
    }

    size_type offset() {
        return pos - begin;
    }

protected:
    boost::iostreams::mapped_file_source::iterator pos;
    boost::iostreams::mapped_file_source::iterator begin;
    boost::iostreams::mapped_file_source::iterator end;
};

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
    unsigned int filePos = 0;

    if (!blockList.empty()) {
        auto &lastBlock = blockList.back();
        fileNum = lastBlock.nFile;
        filePos = lastBlock.nDataPos;
    }

    auto firstFile = fileNum;

    while (true) {
        // determine block file path
        auto blockFilePath = config.pathForBlockFile(fileNum);
        if (!boost::filesystem::exists(blockFilePath)) {
            std::cout << "No block file " << blockFilePath << "\n";
            break;
        }

        // map the block file into memory
        boost::iostreams::mapped_file_source fileMap;
        std::cout << "Reading " << blockFilePath << "...\n";
        fileMap.open(blockFilePath);

        // setup memory reader
        auto reader = SafeMemReader(fileMap);
        auto must = [&reader, &blockFilePath](bool result, const char* operation) {
            if (!result) {
                std::cerr << "Failed to " << operation
                          << " from " << blockFilePath
                          << " at offset " << reader.offset()
                          << ".\n";
                exit(1);
            }
        };

        // logic for resume from last processed block, note offsetAfterLength below
        must(reader.advance(filePos), "advance to resumed file position");
        if (fileNum == firstFile && filePos > 0) {
            uint32_t length;
            must(reader.rewind(sizeof(length)), "rewind length field in resumed block");
            if (!reader.readNext(&length)) {
                continue;
            }
            must(reader.advance(length), "advance after resumed block");
        }

        // read blocks in loop while we can...
        while (true) {
            uint32_t magic;
            if (!reader.readNext(&magic)) {
                break;
            }
            if (magic != config.blockMagic) {
                break;
            }
            uint32_t length;
            uint32_t numTxes;
            CBlockHeader header;
            must(reader.readNext(&length), "read block length");
            auto offsetAfterLength = reader.offset();
            must(reader.readNext(&header), "read block header");
            must(reader.readVariableLengthInteger(&numTxes), "read number of transactions");
            must(reader.reset(offsetAfterLength), "reset offset after length");
            must(reader.advance(length), "advance length");
            blockList.emplace_back(header, fileNum, offsetAfterLength, numTxes, config);
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
