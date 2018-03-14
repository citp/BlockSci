//
//  dedup_address_type.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/7/18.
//

#ifndef dedup_address_type_hpp
#define dedup_address_type_hpp

#include <array>

#define DEDUP_ADDRESS_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA)
#define DEDUP_ADDRESS_TYPE_SET VAL(NONSTANDARD) VAL(PUBKEY) VAL(SCRIPTHASH) VAL(MULTISIG) VAL(NULL_DATA)
namespace blocksci {
    
    
    struct DedupAddressType {
        
        enum Enum {
#define VAL(x) x
            DEDUP_ADDRESS_TYPE_LIST
#undef VAL
        };
#define VAL(x) Enum::x
        static constexpr std::array<Enum, 5> all = {{DEDUP_ADDRESS_TYPE_LIST}};
#undef VAL
        
        static constexpr size_t size = all.size();
    };
}

namespace std {
    template<> struct hash<blocksci::DedupAddressType::Enum> {
        size_t operator()(blocksci::DedupAddressType::Enum val) const {
            return static_cast<size_t>(val);
        }
    };
}

#endif /* dedup_address_type_hpp */
