//
//  chain_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef chain_access_hpp
#define chain_access_hpp

#include "chain_fwd.hpp"

#include <blocksci/chain/raw_transaction.hpp>
#include <blocksci/chain/raw_block.hpp>
#include <blocksci/util/file_mapper.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>

#include <memory>

namespace blocksci {
    
    class ReorgException : public std::runtime_error {
    public:
        ReorgException();
        ReorgException(const ReorgException &) = default;
        ReorgException(ReorgException &&) = default;
        virtual ~ReorgException();
    };
    
    struct DataConfiguration;
    
    class ChainAccess {
        FixedSizeFileMapper<RawBlock> blockFile;
        SimpleFileMapper<> blockCoinbaseFile;
        
        IndexedFileMapper<AccessMode::readonly, RawTransaction> txFile;
        IndexedFileMapper<AccessMode::readonly, uint32_t> sequenceFile;
        
        FixedSizeFileMapper<uint256> txHashesFile;
        
        uint256 lastBlockHash;
        const uint256 *lastBlockHashDisk;
        BlockHeight maxHeight;
        uint32_t _maxLoadedTx;
        BlockHeight blocksIgnored;
        bool errorOnReorg;
        
        void reorgCheck() const {
            if (errorOnReorg && lastBlockHash != *lastBlockHashDisk) {
                throw ReorgException();
            }
        }
        
        void setup();
        
    public:
        ChainAccess(const DataConfiguration &config);
        
        uint32_t maxLoadedTx() const {
            return _maxLoadedTx;
        }
        
        BlockHeight getBlockHeight(uint32_t txIndex) const;
        
        const RawBlock *getBlock(BlockHeight blockHeight) const {
            reorgCheck();
            return blockFile.getData(static_cast<size_t>(static_cast<int>(blockHeight)));
        }
        
        const uint256 *getTxHash(uint32_t index) const {
            reorgCheck();
            return txHashesFile.getData(index);
        }
        
        const RawTransaction *getTx(uint32_t index) const {
            return txFile.getData(index);
        }
        
        const uint32_t *getSequenceNumbers(uint32_t index) const {
            return sequenceFile.getData(index);
        }
        
        size_t txCount() const;
        
        BlockHeight blockCount() const {
            return maxHeight;
        }
        
        std::vector<unsigned char> getCoinbase(uint64_t offset) const;
        
        void reload();
    };
}

#endif /* chain_access_hpp */
