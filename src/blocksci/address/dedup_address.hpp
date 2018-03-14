//
//  dedup_address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#ifndef dedup_address_hpp
#define dedup_address_hpp

#include "address_fwd.hpp"
#include <blocksci/chain/chain_fwd.hpp>

#include <range/v3/utility/optional.hpp>
#include <functional>
#include <vector>

namespace blocksci {
    class DataAccess;
    struct DataConfiguration;
    
    
    struct DedupAddress {
        uint32_t scriptNum;
        DedupAddressType::Enum type;
        
        DedupAddress();
        DedupAddress(uint32_t addressNum, DedupAddressType::Enum type);
        
        bool operator==(const DedupAddress& other) const {
            return type == other.type && scriptNum == other.scriptNum;
        }
        
        bool operator!=(const DedupAddress& other) const {
            return !operator==(other);
        }
        
        std::string toString() const;
    };
    
    inline std::ostream &operator<<(std::ostream &os, const DedupAddress &address) {
        return os << address.toString();
    }
}

namespace std {
    template <>
    struct hash<blocksci::DedupAddress> {
        typedef blocksci::DedupAddress argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.scriptNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}

#endif /* dedup_address_hpp */
