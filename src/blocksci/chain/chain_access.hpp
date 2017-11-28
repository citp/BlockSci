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
        virtual ~ReorgException();
    };
    
    struct DataConfiguration;
    
    class ChainAccess {
        FixedSizeFileMapper<RawBlock> blockFile;
        SimpleFileMapper<> blockCoinbaseFile;
        
        IndexedFileMapper<AccessMode::readonly, RawTransaction> txFile;
        FixedSizeFileMapper<uint256> txHashesFile;
        
        uint256 lastBlockHash;
        const uint256 *lastBlockHashDisk;
        uint32_t maxHeight;
        uint32_t _maxLoadedTx;
        uint32_t blocksIgnored;
        bool errorOnReorg;
        
        void reorgCheck() const {
            if (errorOnReorg && lastBlockHash != *lastBlockHashDisk) {
                throw ReorgException();
            }
        }
        
        void setup();
        
    public:
        ChainAccess(const DataConfiguration &config, bool errorOnReorg, uint32_t blocksIgnored);
        
        uint32_t maxLoadedTx() const;
        
        uint32_t getBlockHeight(uint32_t txIndex) const;
        
        const RawBlock *getBlock(uint32_t blockHeight) const {
            reorgCheck();
            return blockFile.getData(blockHeight);
        }
        
        const uint256 *getTxHash(uint32_t index) const {
            reorgCheck();
            return txHashesFile.getData(index);
        }
        
        const RawTransaction *getTx(uint32_t index) const {
            return txFile.getData(index);
        }
        
        size_t txCount() const;
        
        size_t blockCount() const {
            return maxHeight;
        }
        
        std::vector<unsigned char> getCoinbase(uint64_t offset) const;
        
        void reload();
    };
}

#endif /* chain_access_hpp */
