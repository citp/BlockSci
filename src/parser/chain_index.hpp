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

#include <blocksci/uint256.hpp>

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <fstream>
#include <stdio.h>

struct FileParserConfiguration;
class CBlockIndex;
struct CBlockHeader;

struct BlockInfo {
    //! pointer to the hash of the block, if any. Memory is owned by this CBlockIndex
    blocksci::uint256 hash;
    blocksci::uint256 prevHash;
    
    //! height of the entry in the chain. The genesis block has height 0
    int height;
    
    //! Which # file this block is stored in (blk?????.dat)
    int nFile;
    
    //! Byte offset within blk?????.dat where this block's data is stored
    unsigned int nDataPos;
    
    //! Number of transactions in this block.
    unsigned int nTx;
    
    //! Verification status of this block. See enum BlockStatus
    unsigned int nStatus;
    
    //! block header
    int nVersion;
    blocksci::uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;
    
    BlockInfo() {};
    BlockInfo(const CBlockHeader &h, int fileNum, unsigned int dataPos, unsigned int numTxes, const FileParserConfiguration &config);
};

struct ChainIndex {
    std::vector<BlockInfo> blockList;
    ChainIndex(const FileParserConfiguration &config);
    ChainIndex(const FileParserConfiguration &config_, std::vector<BlockInfo> &blockList_) : blockList(blockList_), config(config_) {}
    
    ~ChainIndex();
    
    const FileParserConfiguration &config;
    
    void updateFromFilesystem();
    
    std::vector<BlockInfo> generateChain(int maxBlockHeight) const;
    
private:
    int updateHeight(size_t blockNum, const std::unordered_map<blocksci::uint256, size_t> &indexMap);
};

std::vector<BlockInfo> generateChain(std::vector<BlockInfo> &blockList, int maxBlockHeight);

#endif

#endif /* data_store_hpp */
