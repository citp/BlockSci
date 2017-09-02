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

#define SCRIPT_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(PUBKEYHASH), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA)
#define SCRIPT_TYPE_SET VAL(NONSTANDARD) VAL(PUBKEY) VAL(PUBKEYHASH) VAL(SCRIPTHASH) VAL(MULTISIG) VAL(NULL_DATA)

namespace blocksci {
    

    struct ScriptType {
        
        enum class Enum {
            #define VAL(x) x
            SCRIPT_TYPE_LIST
            #undef VAL
        };
        #define VAL(x) Enum::x
        static constexpr std::array<Enum, 6> all = {{SCRIPT_TYPE_LIST}};
        #undef VAL
        
        static constexpr size_t size = all.size();
    };
}

std::ostream &operator<<(std::ostream &os, blocksci::ScriptType::Enum const &type);

#endif /* script_types_hpp */
