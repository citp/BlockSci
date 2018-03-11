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

namespace blocksci {
    

    struct AddressType {
        
        enum Enum {
            #define VAL(x) x
            ADDRESS_TYPE_LIST
            #undef VAL
        };
        #define VAL(x) Enum::x
        static constexpr std::array<Enum, 9> all = {{ADDRESS_TYPE_LIST}};
        #undef VAL
        
        static constexpr size_t size = all.size();
    };
}

namespace std {
    template<> struct hash<blocksci::AddressType::Enum> {
        size_t operator()(blocksci::AddressType::Enum type) const {
            return static_cast<size_t>(type);
        }
    };
}

std::ostream &operator<<(std::ostream &os, blocksci::AddressType::Enum const &type);

#endif /* script_types_hpp */
