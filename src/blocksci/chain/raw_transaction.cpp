//
//  raw_transaction.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#include "raw_transaction.hpp"

namespace blocksci {
    RawTransaction::RawTransaction(uint32_t realSize_, uint32_t baseSize_, uint32_t locktime_, uint16_t inputCount_, uint16_t outputCount_) : realSize(realSize_), baseSize(baseSize_), locktime(locktime_), inputCount(inputCount_), outputCount(outputCount_) {}
}
