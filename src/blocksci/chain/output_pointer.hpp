//
//  output_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef output_pointer_hpp
#define output_pointer_hpp

#include "inout_pointer.hpp"

namespace blocksci {
    struct OutputPointer : public InoutPointer {
        using InoutPointer::InoutPointer;
        
        std::string toString() const;
        
        const Output &getOutput(const ChainAccess &access) const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        const Output &getOutput() const;
        #endif
    };
}

std::ostream &operator<<(std::ostream &os, const blocksci::OutputPointer &pointer);

namespace std {
    template<> struct hash<blocksci::OutputPointer> {
        size_t operator()(const blocksci::OutputPointer &pointer) const;
    };
}

#endif /* output_pointer_hpp */
