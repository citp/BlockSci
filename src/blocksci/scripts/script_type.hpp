//
//  script_type.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef script_type_hpp
#define script_type_hpp

#include <array>
#include <stdio.h>

#define SCRIPT_TYPE_LIST VAL(NONSTANDARD), VAL(PUBKEY), VAL(SCRIPTHASH), VAL(MULTISIG), VAL(NULL_DATA)
#define SCRIPT_TYPE_SET VAL(NONSTANDARD) VAL(PUBKEY) VAL(SCRIPTHASH) VAL(MULTISIG) VAL(NULL_DATA)
namespace blocksci {
    
    
    struct ScriptType {
        
        enum class Enum {
#define VAL(x) x
            SCRIPT_TYPE_LIST
#undef VAL
        };
#define VAL(x) Enum::x
        static constexpr std::array<Enum, 5> all = {{SCRIPT_TYPE_LIST}};
#undef VAL
        
        static constexpr size_t size = all.size();
    };
}

#endif /* script_type_hpp */
