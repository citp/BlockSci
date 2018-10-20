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

#include <blocksci/core/typedefs.hpp>
#include <blocksci/core/bitcoin_uint256.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <limits>

class CBlockIndex;
struct blockinfo_t;

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, blocksci::uint256 & g, const unsigned int) {
    ar & g.data;
}

} // namespace serialization
} // namespace boost

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
    blocksci::BlockHeight height;
    
    // Length of block in bytes
    uint32_t size;
    
    //! Number of transactions in this block.
    unsigned int nTx;
    
    uint32_t inputCount;
    uint32_t outputCount;
    
    BlockInfoBase() {}
    BlockInfoBase(const blocksci::uint256 &hash, const CBlockHeader &h, uint32_t size, unsigned int numTxes, uint32_t inputCount, uint32_t outputCount);
    
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & hash;
        ar & header;
        ar & height;
        ar & size;
        ar & nTx;
        ar & inputCount;
        ar & outputCount;
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
    BlockInfo(const CBlockHeader &h, uint32_t length, unsigned int numTxes, uint32_t inputCount, uint32_t outputCount, const ChainDiskConfiguration &config, int fileNum, unsigned int dataPos);
    
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
    BlockInfo(const blockinfo_t &info, blocksci::BlockHeight height);
    
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
    std::unordered_map<blocksci::uint256, BlockType> blockList;
    BlockType newestBlock;
    
    void update(const ConfigType &config);

    std::vector<BlockType> generateChain(blocksci::BlockHeight maxBlockHeight) const {
        std::vector<BlockType> chain;
        const BlockType *maxHeightBlock = nullptr;
        blocksci::BlockHeight maxHeight = std::numeric_limits<blocksci::BlockHeight>::min();
        for (auto &pair : blockList) {
            if (pair.second.height > maxHeight) {
                maxHeightBlock = &pair.second;
                maxHeight = pair.second.height;
            }
        }
        
        if (!maxHeightBlock) {
            return chain;
        }
        
        blocksci::uint256 nullHash;
        nullHash.SetNull();
        
        auto hash = maxHeightBlock->hash;
        
        while (hash != nullHash) {
            auto &block = blockList.find(hash)->second;
            chain.push_back(block);
            hash = block.header.hashPrevBlock;
        }
        
        std::reverse(chain.begin(), chain.end());
        if (maxBlockHeight < 0) {
            return {chain.begin(), chain.end() + maxBlockHeight};
        } else if (maxBlockHeight == 0 || maxBlockHeight > static_cast<blocksci::BlockHeight>(chain.size())) {
            return chain;
        } else {
            return {chain.begin(), chain.begin() + maxBlockHeight};
        }
    }
    
    template <typename GetBlockHash>
    blocksci::BlockHeight findSplitPointIndex(blocksci::BlockHeight blockHeight, GetBlockHash getBlockHash) {
        auto oldBlocks = generateChain(blockHeight);
        
        auto maxSize = std::min(static_cast<blocksci::BlockHeight>(oldBlocks.size()), blockHeight);
        auto splitPoint = maxSize;
        for (blocksci::BlockHeight i{0}; i < maxSize; ++i) {
            blocksci::uint256 oldHash = oldBlocks[static_cast<size_t>(static_cast<int>(maxSize - 1 - i))].hash;
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
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & blockList;
        ar & newestBlock;
    }
    
    int updateHeight(size_t blockNum, const std::unordered_map<blocksci::uint256, size_t> &indexMap);
};

std::vector<BlockInfo<FileTag>> readBlocksInfo(int fileNum, const ParserConfiguration<FileTag> &config);

#endif /* data_store_hpp */
