//
//  address_types.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 2/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blocksci_address_address_types_hpp
#define blocksci_address_address_types_hpp

#include <blocksci/blocksci_export.h>

#include <cstddef>
#include <functional>
#include <tuple>

#define ADDRESS_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(PUBKEYHASH), VAL(MULTISIG_PUBKEY), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA), VAL(WITNESS_PUBKEYHASH), VAL(WITNESS_SCRIPTHASH), VAL(WITNESS_UNKNOWN)

namespace blocksci {
    struct BLOCKSCI_EXPORT AddressType {

        /** enum that holds all address types as listed in ADDRESS_TYPE_LIST */
        enum Enum {
            // After preprocessing: NONSTANDARD, PUBKEY, PUBKEYHASH, MULTISIG_PUBKEY, SCRIPTHASH, MULTISIG, NULL_DATA, WITNESS_PUBKEYHASH, WITNESS_SCRIPTHASH, WITNESS_UNKNOWN
            #define VAL(x) x
            ADDRESS_TYPE_LIST
            #undef VAL
        };
        static constexpr size_t size = 10;

        /* After preprocessing:
         * using all = std::tuple<std::integral_constant<Enum, NONSTANDARD>, std::integral_constant<Enum, PUBKEY>, std::integral_constant<Enum, PUBKEYHASH>, std::integral_constant<Enum, MULTISIG_PUBKEY>, std::integral_constant<Enum, SCRIPTHASH>, std::integral_constant<Enum, MULTISIG>, std::integral_constant<Enum, NULL_DATA>, std::integral_constant<Enum, WITNESS_PUBKEYHASH>, std::integral_constant<Enum, WITNESS_SCRIPTHASH>, std::integral_constant<Enum, WITNESS_UNKNOWN> >;
         */
        #define VAL(x) std::integral_constant<Enum, x>
        using all = std::tuple<ADDRESS_TYPE_LIST>;
        #undef VAL
        
        static constexpr Enum example = PUBKEY;
    };
}

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::AddressType::Enum> {
        size_t operator()(blocksci::AddressType::Enum type) const {
            return static_cast<size_t>(type);
        }
    };
}

#endif /* blocksci_address_address_types_hpp */
