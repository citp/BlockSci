//
//  raw_input.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef raw_input_hpp
#define raw_input_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/input_pointer.hpp>
#include <blocksci/core/inout.hpp>
#include <blocksci/core/raw_transaction.hpp>

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::Input> {
        size_t operator()(const blocksci::Input &input) const;
    };
} // namespace std

namespace blocksci {
    
    class DataAccess;
    
    class BLOCKSCI_EXPORT Input {
        DataAccess *access;
        uint32_t maxTxCount;
        const Inout *inout;
        const uint16_t *spentOutputNum;
        const uint32_t *sequenceNum;
        
        
        friend size_t std::hash<Input>::operator()(const Input &) const;
    public:
        InputPointer pointer;
        
        BlockHeight blockHeight;

        Input(const InputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, const uint16_t *spentOutputNum_, const uint32_t *sequenceNum_, uint32_t maxTxCount_, DataAccess &access_) :
        access(&access_), maxTxCount(maxTxCount_), inout(&inout_), spentOutputNum(spentOutputNum_), sequenceNum(sequenceNum_), pointer(pointer_), blockHeight(blockHeight_) {}
        
        Input(const InputPointer &pointer_, DataAccess &access_);
        
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
        
        AddressType::Enum getType() const {
            return inout->getType();
        }
        
        int64_t getValue() const {
            return inout->getValue();
        }
        
        Address getAddress() const;
        
        uint32_t spentTxIndex() const {
            return inout->getLinkedTxNum();
        }
        
        std::string toString() const;
        
        Transaction getSpentTx() const;
        
        OutputPointer getSpentOutputPointer() const;
        Output getSpentOutput() const;
    };
    
    inline bool BLOCKSCI_EXPORT operator==(const Input& a, const Input& b) {
        return a.pointer == b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator!=(const Input& a, const Input& b) {
        return a.pointer != b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator<(const Input& a, const Input& b) {
        return a.pointer < b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator<=(const Input& a, const Input& b) {
        return a.pointer <= b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator>(const Input& a, const Input& b) {
        return a.pointer > b.pointer;
    }
    
    inline bool BLOCKSCI_EXPORT operator>=(const Input& a, const Input& b) {
        return a.pointer >= b.pointer;
    }

    std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const Input &input);
} // namespace blocksci

namespace std {
    inline size_t hash<blocksci::Input>::operator()(const blocksci::Input &input) const {
        std::size_t seed = 235896754;
        hash_combine(seed, input.pointer);
        return seed;
    }
} // namespace std

#endif /* raw_input_hpp */
