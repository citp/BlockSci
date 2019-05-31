//
//  raw_transaction.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef raw_transaction_hpp
#define raw_transaction_hpp

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/inout.hpp>
#include <blocksci/util/util.hpp>

namespace blocksci {
    struct RawTransaction {
        uint32_t sizeBytes;
        uint32_t locktime;
        uint16_t inputCount;
        uint16_t outputCount;
        InPlaceArray<Inout> inOuts;
        
        RawTransaction(uint32_t sizeBytes, uint32_t locktime, uint16_t inputCount, uint16_t outputCount);
        
        RawTransaction(const RawTransaction &) = delete;
        RawTransaction(RawTransaction &&) = delete;
        RawTransaction &operator=(const RawTransaction &) = delete;
        RawTransaction &operator=(RawTransaction &&) = delete;
        
        Inout &getOutput(uint16_t outputNum) {
            return reinterpret_cast<Inout &>(inOuts[inputCount + outputNum]);
        }
        
        Inout &getInput(uint16_t inputNum) {
            return reinterpret_cast<Inout &>(inOuts[inputNum]);
        }
        
        const Inout &getOutput(uint16_t outputNum) const {
            return reinterpret_cast<const Inout &>(inOuts[inputCount + outputNum]);
        }
        
        const Inout &getInput(uint16_t inputNum) const {
            return reinterpret_cast<const Inout &>(inOuts[inputNum]);
        }
        
        size_t realSize() const {
            return sizeof(RawTransaction) + inOuts.extraSize();
        }
    };
}

#endif /* raw_transaction_hpp */
