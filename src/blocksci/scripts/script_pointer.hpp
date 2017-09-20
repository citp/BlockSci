//
//  script_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/19/17.
//
//

#ifndef script_pointer_hpp
#define script_pointer_hpp

#include "script_type.hpp"

#include <stdio.h>

namespace blocksci {
    struct Address;
    
    struct ScriptPointer {
        uint32_t scriptNum;
        ScriptType::Enum type;
        
        ScriptPointer() {}
        ScriptPointer(uint32_t scriptNum_, ScriptType::Enum type_) : scriptNum(scriptNum_), type(type_) {}
        ScriptPointer(const Address &address);
        
        bool operator==(const ScriptPointer &other) const {
            return scriptNum == other.scriptNum && type == other.type;
        }
    };
}

namespace std {
    template <>
    struct hash<blocksci::ScriptPointer> {
        typedef blocksci::ScriptPointer argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.scriptNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}


#endif /* script_pointer_hpp */
