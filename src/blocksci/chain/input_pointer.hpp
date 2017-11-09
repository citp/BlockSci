//
//  input_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef input_pointer_hpp
#define input_pointer_hpp

#include "inout_pointer.hpp"

namespace blocksci {
    struct InputPointer : public InoutPointer {
        using InoutPointer::InoutPointer;
        
        std::string toString() const;
        
        const Input &getInput(const ChainAccess &access) const;
        Output getOutput(const ChainAccess &access) const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        const Input &getInput() const;
        Output getOutput() const;
        #endif
    };
}

namespace std {
    template<> struct hash<blocksci::InputPointer> {
        size_t operator()(const blocksci::InputPointer &pointer) const;
    };
}

#endif /* input_pointer_hpp */
