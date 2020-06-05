//
//  output.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef output_hpp
#define output_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/core/inout.hpp>
#include <blocksci/core/raw_transaction.hpp>

#include <range/v3/utility/optional.hpp>

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::Output> {
        size_t operator()(const blocksci::Output &output) const;
    };
} // namespace std

namespace blocksci {
    class BLOCKSCI_EXPORT Output {
        DataAccess *access;

        /** Pointer to Inout that represents this Input (Inout consists of linkedTxNum, scriptNum, type, and value) */
        const Inout *inout;

        /** Tx number of the transaction that spends this Output; 0 if still unspent (at the loaded block height) */
        uint32_t spendingTxIndex;

        mutable BlockHeight blockHeight = -1;
        friend size_t std::hash<Output>::operator()(const Output &) const;
    public:
        /** Contains data to uniquely identify one output using txNum and inoutNum */
        OutputPointer pointer;
        
        Output(const OutputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, uint32_t maxTxLoaded, DataAccess &access_) :
        access(&access_), inout(&inout_), blockHeight(blockHeight_), pointer(pointer_) {
            if (inout->getLinkedTxNum() < maxTxLoaded) {
                spendingTxIndex = inout->getLinkedTxNum();
            } else {
                spendingTxIndex = 0;
            }
        }
        
        Output(const OutputPointer &pointer_, DataAccess &access_);
        
        DataAccess &getAccess() const {
            return *access;
        }
        
        BlockHeight getBlockHeight() const;

        /** Get the tx number of the tx that spends this output (in one of its inputs) */
        ranges::optional<uint32_t> getSpendingTxIndex() const {
            return spendingTxIndex > 0 ? ranges::optional<uint32_t>{spendingTxIndex} : ranges::nullopt;
        }
        
        uint32_t txIndex() const {
            return pointer.txNum;
        }

        uint32_t outputIndex() const {
            return pointer.inoutNum;
        }
        
        Transaction transaction() const;
        Block block() const;
        
        bool isSpent() const {
            return spendingTxIndex > 0u;
        }
        
        bool operator==(const Inout &other) const {
            return *inout == other;
        }
        
        blocksci::AddressType::Enum getType() const {
            return inout->getType();
        }
        
        Address getAddress() const;
        
        int64_t getValue() const {
            return inout->getValue();
        }

        std::string toString() const;

        /** Get the Transaction that spends this output, if it was spent yet (at the loaded block height) */
        ranges::optional<Transaction> getSpendingTx() const;

        /** Get the Input that spends this Output */
        ranges::optional<Input> getSpendingInput() const;

        ranges::optional<InputPointer> getSpendingInputPointer() const;
    };
    
    inline bool BLOCKSCI_EXPORT operator==(const Output& a, const Output& b) {
        return a.pointer == b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator!=(const Output& a, const Output& b) {
        return a.pointer != b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator<(const Output& a, const Output& b) {
        return a.pointer < b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator<=(const Output& a, const Output& b) {
        return a.pointer <= b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator>(const Output& a, const Output& b) {
        return a.pointer > b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator>=(const Output& a, const Output& b) {
        return a.pointer >= b.pointer;
    }

    std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const Output &output);
} // namespace blocksci

namespace std {
    inline size_t hash<blocksci::Output>::operator()(const blocksci::Output &output) const {
        std::size_t seed = 819543;
        hash_combine(seed, output.pointer);
        return seed;
    }
} // namespace std

#endif /* output_hpp */
