//
//  dedup_address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#ifndef dedup_address_hpp
#define dedup_address_hpp

#include "address_fwd.hpp"

#include <blocksci/core/dedup_address_info.hpp>
#include <blocksci/chain/chain_fwd.hpp>

#include <functional>
#include <sstream>

namespace blocksci {
    class DataAccess;
    struct DataConfiguration;
    
    
    struct DedupAddress {
        uint32_t scriptNum;
        DedupAddressType::Enum type;
        
        DedupAddress() : scriptNum(0), type(DedupAddressType::NONSTANDARD) {}
        DedupAddress(uint32_t addressNum_, DedupAddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
        
        bool operator==(const DedupAddress& other) const {
            return type == other.type && scriptNum == other.scriptNum;
        }
        
        bool operator!=(const DedupAddress& other) const {
            return !operator==(other);
        }
        
        std::string toString() const {
            if (scriptNum == 0) {
                return "InvalidDedupAddress()";
            } else {
                std::stringstream ss;
                ss << "DedupAddress(";
                ss << "scriptNum=" << scriptNum;
                ss << ", type=" << dedupAddressName(type);
                ss << ")";
                return ss.str();
            }
        }
    };
}

inline std::ostream &operator<<(std::ostream &os, const blocksci::DedupAddress &address) {
    os << address.toString();
    return os;
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
