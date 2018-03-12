//
//  equiv_address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#ifndef equiv_address_hpp
#define equiv_address_hpp

#include "address_fwd.hpp"
#include <blocksci/chain/chain_fwd.hpp>

#include <range/v3/utility/optional.hpp>
#include <functional>
#include <vector>

namespace blocksci {
    class DataAccess;
    struct DataConfiguration;
    
    
    struct EquivAddress {
        uint32_t scriptNum;
        DedupAddressType::Enum type;
        
        EquivAddress();
        EquivAddress(uint32_t addressNum, DedupAddressType::Enum type);
        
        bool operator==(const EquivAddress& other) const {
            return type == other.type && scriptNum == other.scriptNum;
        }
        
        bool operator!=(const EquivAddress& other) const {
            return !operator==(other);
        }
        
        std::string toString() const;
    };
    
    inline std::ostream &operator<<(std::ostream &os, const EquivAddress &address) {
        return os << address.toString();
    }
}

namespace std {
    template <>
    struct hash<blocksci::EquivAddress> {
        typedef blocksci::EquivAddress argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.scriptNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}

#endif /* equiv_address_hpp */
