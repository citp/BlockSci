//
//  inout.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/28/17.
//
//

#ifndef inout_hpp
#define inout_hpp

#include <blocksci/scripts/script_types.hpp>
#include <stdio.h>

namespace blocksci {
    class  ChainAccess;
    struct Transaction;
    struct AddressPointer;
    struct OutputPointer;
    
    struct Inout {
        uint32_t linkedTxNum;
        uint32_t toAddressNum;
        uint64_t other;
        
        Inout(uint32_t linkedTxNum, const AddressPointer &address, uint64_t value);
        
        uint64_t getValue() const {
            uint64_t valueMask = (uint64_t(1) << 60) - 1;
            return other & valueMask;
        }
        
        void setValue(uint64_t value) {
            uint64_t valueMask = (uint64_t(1) << 60) - 1;
            other &= ~valueMask;
            other |= value & valueMask;
        }
        
        ScriptType::Enum getType() const {
            return static_cast<ScriptType::Enum>((other >> 60) & 0b1111);
        }
        
        void setType(ScriptType::Enum type) {
            uint8_t intType = static_cast<uint8_t>(type);
            other &= ~(uint64_t(0b1111) << 60);
            other |= (intType & uint64_t(0b1111)) << 60;
        }
        
        bool operator==(const Inout& other) const;
        
        bool operator!=(const Inout& other) const {
            return ! operator==(other);
        }
        
        AddressPointer getAddressPointer() const;
        
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        
//        static const Inout &create(const OutputPointer &pointer);
        #endif
    };
}

#endif /* inout_hpp */
