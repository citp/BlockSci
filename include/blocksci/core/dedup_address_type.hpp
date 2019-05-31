//
//  dedup_address_type.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/7/18.
//

#ifndef dedup_address_type_hpp
#define dedup_address_type_hpp

#include <blocksci/meta.hpp>

#define DEDUP_ADDRESS_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA)
#define DEDUP_ADDRESS_TYPE_SET VAL(NONSTANDARD) VAL(PUBKEY) VAL(SCRIPTHASH) VAL(MULTISIG) VAL(NULL_DATA)
namespace blocksci {
    
    
    struct DedupAddressType {
        
        enum Enum {
#define VAL(x) x
            DEDUP_ADDRESS_TYPE_LIST
#undef VAL
        };
        static constexpr size_t size = 5;
        
        #define VAL(x) std::integral_constant<Enum, x>
        using all = std::tuple<DEDUP_ADDRESS_TYPE_LIST>;
        #undef VAL
        static constexpr Enum example = PUBKEY;
        
        #define VAL(x) x
        static std::array<Enum,size> allArray() {
            return {{DEDUP_ADDRESS_TYPE_LIST}};
        }
        #undef VAL
    };
    
    template <template<DedupAddressType::Enum> class K>
    using to_dedup_address_tuple_t = apply_template_t<DedupAddressType::Enum, K, DedupAddressType::all>;
}

namespace std {
    template<> struct hash<blocksci::DedupAddressType::Enum> {
        size_t operator()(blocksci::DedupAddressType::Enum val) const {
            return static_cast<size_t>(val);
        }
    };
}

#endif /* dedup_address_type_hpp */
