//
//  raw_block.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#include "raw_block.hpp"

namespace blocksci {
    RawBlock::RawBlock(uint32_t firstTxIndex_, uint32_t numTxes_, uint32_t height_, uint256 hash_, int32_t version_, uint32_t timestamp_, uint32_t bits_, uint32_t nonce_, uint32_t realSize_, uint32_t baseSize_, uint64_t coinbaseOffset_) : firstTxIndex(firstTxIndex_), numTxes(numTxes_), height(height_), hash(hash_), version(version_), timestamp(timestamp_), bits(bits_), nonce(nonce_), realSize(realSize_), baseSize(baseSize_), coinbaseOffset(coinbaseOffset_) {}
    
    bool RawBlock::operator==(const RawBlock& other) const {
        return firstTxIndex == other.firstTxIndex
        && numTxes == other.numTxes
        && height == other.height
        && hash == other.hash
        && version == other.version
        && timestamp == other.timestamp
        && bits == other.bits
        && nonce == other.nonce
        && coinbaseOffset == other.coinbaseOffset;
    }
}
