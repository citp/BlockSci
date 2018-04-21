//
//  raw_transaction.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef raw_transaction_hpp
#define raw_transaction_hpp

#include <blocksci/blocksci_export.h>
#include "chain_fwd.hpp"
#include "inout.hpp"

#include <range/v3/iterator_range.hpp>

namespace blocksci {
    struct BLOCKSCI_EXPORT RawTransaction {
        uint32_t realSize;
        uint32_t baseSize;
        uint32_t locktime;
        uint16_t inputCount;
        uint16_t outputCount;
        
        RawTransaction(uint32_t realSize_, uint32_t baseSize_, uint32_t locktime_, uint16_t inputCount_, uint16_t outputCount_) : realSize(realSize_), baseSize(baseSize_), locktime(locktime_), inputCount(inputCount_), outputCount(outputCount_) {}
        
        RawTransaction(const RawTransaction &) = delete;
        RawTransaction(RawTransaction &&) = delete;
        RawTransaction &operator=(const RawTransaction &) = delete;
        RawTransaction &operator=(RawTransaction &&) = delete;
        
        Inout &getOutput(uint16_t outputNum) {
            return getInouts()[inputCount + outputNum];
        }
        
        Inout &getInput(uint16_t inputNum) {
            return getInouts()[inputNum];
        }
        
        const Inout &getOutput(uint16_t outputNum) const {
            return getInouts()[inputCount + outputNum];
        }
        
        const Inout &getInput(uint16_t inputNum) const {
            return getInouts()[inputNum];
        }
        
        ranges::iterator_range<const Inout *> outputs() const {
            auto &firstOut = getOutput(0);
            return ranges::make_iterator_range(&firstOut, &firstOut + outputCount);
        }
        
        ranges::iterator_range<const Inout *> inputs() const {
            auto &firstIn = getInput(0);
            return ranges::make_iterator_range(&firstIn, &firstIn + inputCount);
        }
        
        size_t serializedSize() const {
            return sizeof(RawTransaction) + sizeof(Inout) * (inputCount + outputCount);
        }
        
    private:
        const Inout *getInouts() const {
            return reinterpret_cast<const Inout *>(this + 1);
        }
        
        Inout *getInouts() {
            return reinterpret_cast<Inout *>(this + 1);
        }
    };
} // namespace blocksci

#endif /* raw_transaction_hpp */
