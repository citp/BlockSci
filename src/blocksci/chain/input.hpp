//
//  raw_input.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef raw_input_hpp
#define raw_input_hpp

#include "inout.hpp"
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>

#include <cstdint>
#include <string>

namespace std {
    template<> struct hash<blocksci::Input> {
        size_t operator()(const blocksci::Input &input) const;
    };
}

namespace blocksci {
    struct Address;
    
    class Input {
        const ChainAccess *access;
        const Inout *inout;
        
        friend size_t std::hash<Input>::operator()(const Input &) const;
    public:
        Input(const Inout &inout_, const ChainAccess &access_) : access(&access_), inout(&inout_) {}
        Input(const InputPointer &pointer, const ChainAccess &access_);
        
        bool operator==(const Inout &other) const {
            return *inout == other;
        }
        
        bool operator==(const Input &other) const {
            return *inout == *other.inout;
        }
        
        AddressType::Enum getType() const {
            return inout->getType();
        }
        
        uint64_t getValue() const {
            return inout->getValue();
        }
        
        Address getAddress() const {
            return inout->getAddress();
        }
        
        uint32_t spentTxIndex() const {
            return inout->linkedTxNum;
        }
        
        
        std::string toString() const;
        Transaction getSpentTx() const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Input(const InputPointer &pointer);
        #endif
    };
}



std::ostream &operator<<(std::ostream &os, blocksci::Input const &input);

#endif /* raw_input_hpp */
