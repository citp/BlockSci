//
//  output.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef output_hpp
#define output_hpp

#include "inout.hpp"
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
        
        friend size_t std::hash<Output>::operator()(const Output &) const;
    public:
        Output(const Inout &inout_, const ChainAccess &access_) : access(&access_), inout(&inout_) {}
        Output(const OutputPointer &pointer, const ChainAccess &access_);
        
        uint32_t getSpendingTxIndex() const;
        
        bool isSpent() const {
            return getSpendingTxIndex() != 0;
        }
        
        bool operator==(const Output &other) const;
        
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
}

std::ostream &operator<<(std::ostream &os, blocksci::Output const &output);

#endif /* output_hpp */
