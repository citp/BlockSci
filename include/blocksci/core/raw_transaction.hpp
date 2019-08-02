//
//  raw_transaction.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef raw_transaction_hpp
#define raw_transaction_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/core/inout.hpp>
#include <blocksci/core/typedefs.hpp>

namespace blocksci {
    /** Represents raw transaction data as stored in the chain/tx_data.dat file.
     *
     * The chain/tx_data.dat file is implemented as IndexedFileMapper<mio::access_mode, RawTransaction> and is
     * accessible via ChainAccess' txFile property.
     *
     * Actual inputs and outputs are not part of the RawTransaction struct, but are stored as Inout objects
     * in the chain/tx_data.dat file, @see blocksci::ChainAccess for details.
     */
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

        const Inout *beginInputs() const {
            return &getInput(0);
        }

        const Inout *endInputs() const {
            return &getInput(0) + inputCount;
        }

        const Inout *beginOutputs() const {
            return &getOutput(0);
        }

        const Inout *endOutputs() const {
            return &getOutput(0) + outputCount;
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
