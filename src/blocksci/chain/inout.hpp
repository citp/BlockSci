//
//  inout.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/28/17.
//
//

#ifndef inout_hpp
#define inout_hpp

#include <blocksci/address/address_types.hpp>

namespace blocksci {
    struct Inout;
}

namespace std
{
    template<> struct hash<blocksci::Inout> {
        size_t operator()(const blocksci::Inout &inout) const;
    };
}

namespace blocksci {
    class  ChainAccess;
    struct Transaction;
    struct Address;
    struct OutputPointer;
    
    struct Inout {
        uint32_t linkedTxNum;
        uint32_t toAddressNum;
        uint64_t other;
        
        void setValue(uint64_t value) {
            uint64_t valueMask = (uint64_t(1) << 60) - 1;
            other &= ~valueMask;
            other |= value & valueMask;
        }
        
        void setType(AddressType::Enum type) {
            uint8_t intType = static_cast<uint8_t>(type);
            other &= ~(uint64_t(0b1111) << 60);
            other |= (intType & uint64_t(0b1111)) << 60;
        }
        
        friend struct std::hash<blocksci::Inout>;
        
        Inout(uint32_t linkedTxNum, const Address &address, uint64_t value);
        
        uint64_t getValue() const {
            uint64_t valueMask = (uint64_t(1) << 60) - 1;
            return other & valueMask;
        }
        
        AddressType::Enum getType() const {
            return static_cast<AddressType::Enum>((other >> 60) & 0b1111);
        }
        
        bool operator==(const Inout& otherInout) const;
        
        bool operator!=(const Inout& otherInout) const {
            return ! operator==(otherInout);
        }
        
        Address getAddress() const;
    };
}

#endif /* inout_hpp */
