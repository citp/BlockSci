//
//  inout.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/28/17.
//
//

#ifndef inout_hpp
#define inout_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/core/address_types.hpp>
#include <blocksci/core/hash_combine.hpp>
#include <blocksci/core/typedefs.hpp>

namespace blocksci {
    struct Inout;
}

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::Inout> {
        size_t operator()(const blocksci::Inout &inout) const;
    };
} // namespace std

namespace blocksci {

    /** Represents an input or output with the following data:
     *     - linked tx num, addressNum (=scriptNum), address type, value of the input/output
     *
     * Actual Input and Output objects can be constructed from Inout objects @see blocksci::Transaction::outputs() and blocksci::Transaction::inputs()
     */
    struct BLOCKSCI_EXPORT Inout {
        Inout(uint32_t linkedTxNum_, uint32_t addressNum, AddressType::Enum type, int64_t value)  : linkedTxNum(linkedTxNum_), toAddressNum(addressNum), other(0) {
            setValue(value);
            setType(type);
        }
        Inout() : linkedTxNum(0), toAddressNum(0), other(0) {}
        
        void setValue(int64_t value) {
            uint64_t valueMask = (uint64_t(1) << 60) - 1;
            // (Re)set 60 lowest bits to 0
            other &= ~valueMask;
            // Set 60 lowest bits to the passed value
            other |= static_cast<uint64_t>(value) & valueMask;
        }
        
        void setType(AddressType::Enum type) {
            uint8_t intType = static_cast<uint8_t>(type);
            // (Re)set 4 highest significant bits to 0
            other &= ~(uint64_t(0b1111) << 60);
            // Set highest 4 bits to the passed type
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
        /** Number of the transaction associated with this input or output:
         *     - for inputs, references back to the tx number of the tx that contains the output spent by the input
         *     - for outputs, references forward to the tx number of the tx that contains the spending input, if any
         */
        uint32_t linkedTxNum;

        /** Number of the address (scriptNum) that is contained in this input/output */
        uint32_t toAddressNum;

        /** Other is used to store the value (60 bit) and the address type (4 bit) of the wrapped input/output */
        uint64_t other;
        
        friend size_t std::hash<Inout>::operator()(const Inout &inout) const;
    };
} // namespace blocksci

namespace std {
    inline size_t hash<blocksci::Inout>::operator()(const blocksci::Inout &inout) const {
        std::size_t seed = 65246342;
        blocksci::hash_combine(seed, inout.linkedTxNum);
        blocksci::hash_combine(seed, inout.toAddressNum);
        blocksci::hash_combine(seed, inout.other);
        return seed;
    }
} // namespace std

#endif /* inout_hpp */
