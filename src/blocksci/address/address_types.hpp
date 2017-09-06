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
#include <stdio.h>

#define ADDRESS_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(PUBKEYHASH), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA), VAL(WITNESS_PUBKEYHASH), VAL(WITNESS_SCRIPTHASH)
#define ADDRESS_TYPE_SET VAL(NONSTANDARD) VAL(PUBKEY) VAL(PUBKEYHASH) VAL(SCRIPTHASH) VAL(MULTISIG) VAL(NULL_DATA) VAL(WITNESS_PUBKEYHASH) VAL(WITNESS_SCRIPTHASH)

namespace blocksci {
    

    struct AddressType {
        
        enum class Enum {
            #define VAL(x) x
            ADDRESS_TYPE_LIST
            #undef VAL
        };
        #define VAL(x) Enum::x
        static constexpr std::array<Enum, 8> all = {{ADDRESS_TYPE_LIST}};
        #undef VAL
        
        static constexpr size_t size = all.size();
    };
}

std::ostream &operator<<(std::ostream &os, blocksci::AddressType::Enum const &type);

#endif /* script_types_hpp */
