//
//  raw_transaction.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#include "raw_transaction.hpp"

namespace blocksci {
    RawTransaction::RawTransaction(uint32_t sizeBytes_, uint32_t locktime_, uint16_t inputCount_, uint16_t outputCount_) : sizeBytes(sizeBytes_), locktime(locktime_), inputCount(inputCount_), outputCount(outputCount_), inOuts(static_cast<uint32_t>(inputCount_ + outputCount_)) {}
}
