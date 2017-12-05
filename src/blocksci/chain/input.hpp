//
//  raw_input.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef raw_input_hpp
#define raw_input_hpp

#include <blocksci/chain/inout.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/chain_access.hpp>
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
        InputPointer pointer;
        
        friend size_t std::hash<Input>::operator()(const Input &) const;
    public:
        BlockHeight blockHeight;

        Input(const InputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, const ChainAccess &access_) : 
        access(&access_), inout(&inout_), pointer(pointer_), blockHeight(blockHeight_) {
            assert(pointer.isValid(access_));
        }
        Input(const InputPointer &pointer_, const ChainAccess &access_) : 
        Input(pointer_, access_.getBlockHeight(pointer_.txNum), access_.getTx(pointer_.txNum)->getInput(pointer_.inoutNum), access_) {}
        

        uint32_t txIndex() const {
            return pointer.txNum;
        }

        uint32_t inputIndex() const {
            return pointer.inoutNum;
        }

        bool operator==(const Inout &other) const {
            return *inout == other;
        }
        
        bool operator==(const Input &other) const {
            return *inout == *other.inout;
        }

        bool operator!=(const Input &other) const {
            return !(*inout == *other.inout);
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
