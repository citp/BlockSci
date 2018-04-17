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
#include "inout_pointer.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/util/data_access.hpp>


namespace std {
    template<> struct hash<blocksci::Input> {
        size_t operator()(const blocksci::Input &input) const;
    };
} // namespace std

namespace blocksci {
    
    class Input {
        DataAccess *access;
        const Inout *inout;
        const uint32_t *sequenceNum;
        InputPointer pointer;
        
        friend size_t std::hash<Input>::operator()(const Input &) const;
    public:
        
        BlockHeight blockHeight;

        Input(const InputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, const uint32_t *sequenceNum_, DataAccess &access_) :
        access(&access_), inout(&inout_), sequenceNum(sequenceNum_), pointer(pointer_), blockHeight(blockHeight_) {
            assert(pointer.isValid(access_.chain));
        }
        Input(const InputPointer &pointer_, DataAccess &access_) :
        Input(pointer_, access_.chain.getBlockHeight(pointer_.txNum), access_.chain.getTx(pointer_.txNum)->getInput(pointer_.inoutNum), &access_.chain.getSequenceNumbers(pointer_.txNum)[pointer_.inoutNum], access_) {}
        
        DataAccess &getAccess() const {
            return *access;
        }
        
        uint32_t txIndex() const {
            return pointer.txNum;
        }

        uint32_t inputIndex() const {
            return pointer.inoutNum;
        }
        
        uint32_t sequenceNumber() const {
            return *sequenceNum;
        }
        
        Transaction transaction() const;
        Block block() const;
        
        BlockHeight age() const;

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
        
        int64_t getValue() const {
            return inout->getValue();
        }
        
        Address getAddress() const {
            return {inout->getAddressNum(), inout->getType(), *access};
        }
        
        uint32_t spentTxIndex() const {
            return inout->getLinkedTxNum();
        }
        
        std::string toString() const;
        
        Transaction getSpentTx() const;
    };

    inline std::ostream &operator<<(std::ostream &os, const Input &input) { 
        return os << input.toString();
    }
} // namespace blocksci

namespace std {
    inline size_t hash<blocksci::Input>::operator()(const blocksci::Input &input) const {
        std::size_t seed = 235896754;
        hash_combine(seed, input.pointer);
        return seed;
    }
} // namespace std

#endif /* raw_input_hpp */
