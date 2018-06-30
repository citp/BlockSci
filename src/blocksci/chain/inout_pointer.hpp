//
//  inout_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef inout_pointer_hpp
#define inout_pointer_hpp

#include <cstdint>
#include <string>
#include <stdio.h>

namespace blocksci {
    struct InoutPointer;
}

namespace blocksci {
    struct Transaction;
    struct Input;
    class ChainAccess;
    
    struct InoutPointer {
        uint32_t txNum;
        uint16_t inoutNum;
        
        InoutPointer() : txNum(0), inoutNum(0) {}
        InoutPointer(uint32_t txNum_, uint16_t inoutNum_) : txNum(txNum_), inoutNum(inoutNum_) {}
        
        Transaction getTransaction(const ChainAccess &access) const;
        
        bool operator==(const InoutPointer& other) const {
            return txNum == other.txNum && inoutNum == other.inoutNum;
        }
        
        bool operator<(const InoutPointer& other) const {
            if (txNum != other.txNum) {
                return txNum < other.txNum;
            } else {
                return inoutNum < other.inoutNum;
            }
        }
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction getTransaction() const;
        #endif
    };
}

#endif /* inout_pointer_hpp */
