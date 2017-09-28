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
#include "parser_fwd.hpp"

#include <blocksci/bitcoin_uint256.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <fstream>
#include <stdio.h>

class CBlockIndex;
struct blockinfo_t;

struct CBlockHeader {
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & nVersion;
        ar & hashPrevBlock;
        ar & hashMerkleRoot;
        ar & nTime;
        ar & nBits;
        ar & nNonce;
    }
    
    // header
    int32_t nVersion;
    blocksci::uint256 hashPrevBlock;
    blocksci::uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
};

struct BlockInfoBase {
    blocksci::uint256 hash;
    
    //! block header
    CBlockHeader header;
    
    //! height of the entry in the chain. The genesis block has height 0
    int height;
    
    // Length of block in bytes
    uint32_t size;
    
    //! Number of transactions in this block.
    unsigned int nTx;
    
    BlockInfoBase() {}
    BlockInfoBase(const blocksci::uint256 &hash, const CBlockHeader &h, uint32_t size, unsigned int numTxes);
    
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & hash;
        ar & header;
        ar & height;
        ar & size;
        ar & nTx;
    }
};

template <typename ParseType>
struct BlockInfo;

template <>
struct BlockInfo<FileTag> : BlockInfoBase {
    
    //! Which # file this block is stored in (blk?????.dat)
    int nFile;
    
    //! Byte offset within blk?????.dat where this block's data is stored
    unsigned int nDataPos;
    
    BlockInfo() : BlockInfoBase() {}
    BlockInfo(const CBlockHeader &h, uint32_t length, unsigned int numTxes, const ParserConfiguration<FileTag> &config, int fileNum, unsigned int dataPos);
    
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<BlockInfoBase>(*this);
        ar & nFile;
        ar & nDataPos;
    }
};

template<>
struct BlockInfo<RPCTag> : BlockInfoBase {
    std::vector<std::string> tx;
    
    BlockInfo() : BlockInfoBase() {}
    BlockInfo(const blockinfo_t &info, uint32_t height);
    
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<BlockInfoBase>(*this);
        ar & tx;
    }
};

template <typename ParseTag>
struct ChainIndex {
    using BlockType = BlockInfo<ParseTag>;
    using ConfigType = ParserConfiguration<ParseTag>;
    std::vector<BlockType> blockList;
    
    ChainIndex(const ConfigType &config_) : config(config_) {
        boost::filesystem::ifstream file(config.blockListPath(), std::ios::binary);
        if (file.good()) {
            boost::archive::binary_iarchive ia(file);
            ia >> *this;
        }
        update();
    }
    
    ~ChainIndex() {
        boost::filesystem::ofstream file(config.blockListPath(), std::ios::binary);
        boost::archive::binary_oarchive oa(file);
        oa << *this;
    }
    
    ChainIndex(const ChainIndex &) = delete;
    
    const ConfigType &config;
    
    void update();
    std::vector<BlockType> generateChain(uint32_t maxBlockHeight) const;
    
    template <typename GetBlockHash>
    uint32_t findSplitPointIndex(uint32_t blockHeight, GetBlockHash getBlockHash) {
        auto oldBlocks = generateChain(blockHeight);
        
        uint32_t maxSize = std::min(static_cast<uint32_t>(oldBlocks.size()), blockHeight);
        uint32_t splitPoint = static_cast<uint32_t>(maxSize);
        for (uint32_t i = 0; i < maxSize; i++) {
            blocksci::uint256 oldHash = oldBlocks[maxSize - 1 - i].hash;
            blocksci::uint256 newHash = getBlockHash(maxSize - 1 - i);
            if (oldHash == newHash) {
                splitPoint = maxSize - i;
                break;
            }
        }
        return splitPoint;
    }
    
private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & blockList;
    }
    
    int updateHeight(size_t blockNum, const std::unordered_map<blocksci::uint256, size_t> &indexMap);
};

#endif /* data_store_hpp */
