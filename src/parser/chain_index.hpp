//
//  data_store.hpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef data_store_hpp
#define data_store_hpp

#include "config.hpp"

#ifdef BLOCKSCI_FILE_PARSER

#include <blocksci/bitcoin_uint256.hpp>

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <fstream>
#include <stdio.h>

struct FileParserConfiguration;
class CBlockIndex;

struct CBlockHeader {
    // header
    int32_t nVersion;
    blocksci::uint256 hashPrevBlock;
    blocksci::uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
};

struct BlockInfo {
    blocksci::uint256 hash;
    
    //! height of the entry in the chain. The genesis block has height 0
    int height;
    
    //! Which # file this block is stored in (blk?????.dat)
    int nFile;
    
    //! Byte offset within blk?????.dat where this block's data is stored
    unsigned int nDataPos;
    
    // Length of block in bytes
    uint32_t length;
    
    //! Number of transactions in this block.
    unsigned int nTx;
    
    //! block header
    CBlockHeader header;
    
    BlockInfo() {}
    BlockInfo(const CBlockHeader &h, int fileNum, unsigned int dataPos, uint32_t length, unsigned int numTxes, const FileParserConfiguration &config);
};

struct ChainIndex {
    std::vector<BlockInfo> blockList;
    ChainIndex(const FileParserConfiguration &config);
    ChainIndex(const FileParserConfiguration &config_, std::vector<BlockInfo> &blockList_) : blockList(blockList_), config(config_) {}
    
    ~ChainIndex();
    
    const FileParserConfiguration &config;
    
    void updateFromFilesystem();
    
    std::vector<BlockInfo> generateChain(uint32_t maxBlockHeight) const;
    
private:
    int updateHeight(size_t blockNum, const std::unordered_map<blocksci::uint256, size_t> &indexMap);
};

std::vector<BlockInfo> generateChain(std::vector<BlockInfo> &blockList, int maxBlockHeight);

#endif

#endif /* data_store_hpp */
