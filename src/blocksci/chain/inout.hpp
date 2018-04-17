//
//  inout.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/28/17.
//
//

#ifndef inout_hpp
#define inout_hpp

#include "chain_fwd.hpp"
#include <blocksci/address/address_types.hpp>
#include <blocksci/util/hash.hpp>

namespace std {
    template<> struct hash<blocksci::Inout> {
        size_t operator()(const blocksci::Inout &inout) const;
    };
} // namespace std

namespace blocksci {
    struct Inout {
        Inout(uint32_t linkedTxNum_, uint32_t addressNum, AddressType::Enum type, int64_t value)  : linkedTxNum(linkedTxNum_), toAddressNum(addressNum), other(0) {
            setValue(value);
            setType(type);
        }
        Inout() : linkedTxNum(0), toAddressNum(0), other(0) {}
        
        void setValue(int64_t value) {
            uint64_t valueMask = (uint64_t(1) << 60) - 1;
            other &= ~valueMask;
            other |= static_cast<uint64_t>(value) & valueMask;
        }
        
        void setType(AddressType::Enum type) {
            uint8_t intType = static_cast<uint8_t>(type);
            other &= ~(uint64_t(0b1111) << 60);
            other |= (intType & uint64_t(0b1111)) << 60;
        }
        
        int64_t getValue() const {
            uint64_t valueMask = (uint64_t(1) << 60) - 1;
            return static_cast<int64_t>(other & valueMask);
        }
        
        AddressType::Enum getType() const {
            return static_cast<AddressType::Enum>((other >> 60) & 0b1111);
        }
        
        uint32_t getLinkedTxNum() const {
            return linkedTxNum;
        }
        
        void setLinkedTxNum(uint32_t val) {
            linkedTxNum = val;
        }
        
        uint32_t getAddressNum() const {
            return toAddressNum;
        }
        
        bool operator==(const Inout& otherInout) const {
            return linkedTxNum == otherInout.linkedTxNum && toAddressNum == otherInout.toAddressNum && other == otherInout.other;
        }
        
        bool operator!=(const Inout& otherInout) const {
            return ! operator==(otherInout);
        }
        
    private:
        uint32_t linkedTxNum;
        uint32_t toAddressNum;
        uint64_t other;
        
        friend size_t std::hash<Inout>::operator()(const Inout &inout) const;
    };
} // namespace blocksci

namespace std {
    inline size_t hash<blocksci::Inout>::operator()(const blocksci::Inout &inout) const {
        std::size_t seed = 65246342;
        hash_combine(seed, inout.linkedTxNum);
        hash_combine(seed, inout.toAddressNum);
        hash_combine(seed, inout.other);
        return seed;
    }
} // namespace std

#endif /* inout_hpp */
