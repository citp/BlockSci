//
//  raw_address.hpp
//  bitcoinapi
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef raw_address_hpp
#define raw_address_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/core/address_types.hpp>

#include <functional>

namespace blocksci {
    struct BLOCKSCI_EXPORT RawAddress {
        uint32_t scriptNum;
        AddressType::Enum type;
        
        RawAddress() {}
        RawAddress(uint32_t addressNum_, AddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
        
        bool operator==(const RawAddress &other) const {
            return scriptNum == other.scriptNum && type == other.type;
        }
    };
} // namespace blocksci

namespace std {
    template <>
    struct BLOCKSCI_EXPORT hash<blocksci::RawAddress> {
        typedef blocksci::RawAddress argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.scriptNum) << 32) + static_cast<size_t>(b.type);
        }
    };
} // namespace std

#endif /* raw_address_hpp */
