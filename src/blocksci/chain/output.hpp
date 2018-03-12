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
#include <blocksci/util/data_access.hpp>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>

#include <range/v3/utility/optional.hpp>

namespace std {
    template<> struct hash<blocksci::Output> {
        size_t operator()(const blocksci::Output &input) const;
    };
}

namespace blocksci {
    
    class Output {
        const DataAccess *access;
        const Inout *inout;
        uint32_t spendingTxIndex;
        
        friend size_t std::hash<Output>::operator()(const Output &) const;
    public:
        OutputPointer pointer;
        BlockHeight blockHeight;
        Output(const OutputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, const DataAccess &access_) :
        access(&access_), inout(&inout_), pointer(pointer_), blockHeight(blockHeight_) {
            assert(pointer.isValid(*access_.chain));
            if (inout->linkedTxNum < access->chain->maxLoadedTx()) {
                spendingTxIndex = inout->linkedTxNum;
            } else {
                spendingTxIndex = 0;
            }
        }
        
        Output(const OutputPointer &pointer_, const DataAccess &access_) :
        Output(pointer_, access_.chain->getBlockHeight(pointer_.txNum), access_.chain->getTx(pointer_.txNum)->getOutput(pointer_.inoutNum), access_) {}
        
        uint32_t getSpendingTxIndex() const {
            return spendingTxIndex;
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
        
        Address getAddress() const {
            return Address(inout->toAddressNum, inout->getType(), *access);
        }
        
        uint64_t getValue() const {
            return inout->getValue();
        }

        std::string toString() const;
        ranges::optional<Transaction> getSpendingTx() const;
    };

    inline std::ostream &operator<<(std::ostream &os, const Output &output) { 
        return os << output.toString();
    }
}

#endif /* output_hpp */
