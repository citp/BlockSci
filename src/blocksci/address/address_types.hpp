//
//  script_types.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 2/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef script_types_hpp
#define script_types_hpp

#include <array>
#include <string>

#define ADDRESS_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(PUBKEYHASH), VAL(MULTISIG_PUBKEY), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA), VAL(WITNESS_PUBKEYHASH), VAL(WITNESS_SCRIPTHASH)
#define ADDRESS_TYPE_SET VAL(NONSTANDARD) VAL(PUBKEY) VAL(PUBKEYHASH) VAL(MULTISIG_PUBKEY) VAL(SCRIPTHASH) VAL(MULTISIG) VAL(NULL_DATA) VAL(WITNESS_PUBKEYHASH) VAL(WITNESS_SCRIPTHASH)

#define EQUIV_ADDRESS_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA)
#define EQUIV_ADDRESS_TYPE_SET VAL(NONSTANDARD) VAL(PUBKEY) VAL(SCRIPTHASH) VAL(MULTISIG) VAL(NULL_DATA)

namespace blocksci {
    struct AddressType {
        
        enum Enum {
            #define VAL(x) x
            ADDRESS_TYPE_LIST
            #undef VAL
        };
        static constexpr size_t size = 9;
        #define VAL(x) std::integral_constant<Enum, x>
        using all = std::tuple<ADDRESS_TYPE_LIST>;
        #undef VAL
        
        static constexpr Enum example = PUBKEY;
    };
    
    struct EquivAddressType {
        
        enum Enum {
            #define VAL(x) x
            EQUIV_ADDRESS_TYPE_LIST
            #undef VAL
        };
        
        static constexpr size_t size = 5;
        #define VAL(x) std::integral_constant<Enum, x>
        using all = std::tuple<EQUIV_ADDRESS_TYPE_LIST>;
        #undef VAL
        static constexpr Enum example = PUBKEY;
    };
}

namespace std {
    template<> struct hash<blocksci::AddressType::Enum> {
        size_t operator()(blocksci::AddressType::Enum type) const {
            return static_cast<size_t>(type);
        }
    };
    
    template<> struct hash<blocksci::EquivAddressType::Enum> {
        size_t operator()(blocksci::EquivAddressType::Enum type) const {
            return static_cast<size_t>(type);
        }
    };
}

#endif /* script_types_hpp */
