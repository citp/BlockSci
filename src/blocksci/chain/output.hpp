//
//  output.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef output_hpp
#define output_hpp

#include <blocksci/chain/inout.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>

#include <range/v3/utility/optional.hpp>

namespace std {
    template<> struct hash<blocksci::Output> {
        size_t operator()(const blocksci::Output &input) const;
    };
}

namespace blocksci {
    
    struct Address;
    
    class Output {
        const ChainAccess *access;
        const Inout *inout;
        uint32_t spendingTxIndex;
        OutputPointer pointer;
        
        friend size_t std::hash<Output>::operator()(const Output &) const;
    public:
        BlockHeight blockHeight;
        Output(const OutputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, const ChainAccess &access_) :
        access(&access_), inout(&inout_), pointer(pointer_), blockHeight(blockHeight_) {
            assert(pointer.isValid(access_));
            if (inout->linkedTxNum < access->maxLoadedTx()) {
                spendingTxIndex = inout->linkedTxNum;
            } else {
                spendingTxIndex = 0;
            }
        }
        
        Output(const OutputPointer &pointer_, const ChainAccess &access_) :
        Output(pointer_, access_.getBlockHeight(pointer_.txNum), access_.getTx(pointer_.txNum)->getOutput(pointer_.inoutNum), access_) {}
        
        uint32_t getSpendingTxIndex() const {
            return spendingTxIndex;
        }

        uint32_t txIndex() const {
            return pointer.txNum;
        }

        uint32_t outputIndex() const {
            return pointer.inoutNum;
        }
        
        bool isSpent() const {
            return getSpendingTxIndex() != 0;
        }
        
        bool operator==(const Output &other) const {
            return pointer == other.pointer;
        }

        bool operator!=(const Output &other) const {
            return pointer != other.pointer;
        }
        
        bool operator==(const Inout &other) const {
            return *inout == other;
        }
        
        blocksci::AddressType::Enum getType() const {
            return inout->getType();
        }
        
        uint64_t getValue() const {
            return inout->getValue();
        }
        
        Address getAddress() const {
            return inout->getAddress();
        }

        std::string toString() const;
        ranges::optional<Transaction> getSpendingTx() const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Output(const OutputPointer &pointer);
        #endif
    };

    inline std::ostream &operator<<(std::ostream &os, const Output &output) { 
        return os << output.toString();
    }
}

#endif /* output_hpp */
