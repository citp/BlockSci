//
//  raw_block.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef raw_block_hpp
#define raw_block_hpp

#include <blocksci/util/bitcoin_uint256.hpp>

namespace blocksci {
    struct RawBlock {
        uint32_t firstTxIndex;
        uint32_t numTxes;
        uint32_t height;
        uint256 hash;
        int32_t version;
        uint32_t timestamp;
        uint32_t bits;
        uint32_t nonce;
        uint32_t realSize;
        uint32_t baseSize;
        uint64_t coinbaseOffset;
        
        RawBlock(uint32_t firstTxIndex_, uint32_t numTxes_, uint32_t height_, uint256 hash_, int32_t version_, uint32_t timestamp_, uint32_t bits_, uint32_t nonce_, uint32_t realSize_, uint32_t baseSize_, uint64_t coinbaseOffset_);
        
        bool operator==(const RawBlock& other) const;
    };
} // namespace blocksci

#endif /* raw_block_hpp */
