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
        uint32_t realSize;
        uint32_t baseSize;
        uint32_t locktime;
        uint16_t inputCount;
        uint16_t outputCount;
        
        RawTransaction(uint32_t realSize, uint32_t baseSize, uint32_t locktime, uint16_t inputCount, uint16_t outputCount);
        
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
}

#endif /* raw_transaction_hpp */
