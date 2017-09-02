//
//  output_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef output_pointer_hpp
#define output_pointer_hpp

#include <cstdint>
#include <string>
#include <stdio.h>

namespace blocksci {
    struct Transaction;
    struct Output;
    class ChainAccess;
    
    struct OutputPointer {
        
        uint32_t txNum;
        uint16_t outputNum;
        
        OutputPointer() : txNum(0), outputNum(0) {}
        OutputPointer(uint32_t txNum, uint16_t outputNum);
        
        bool operator==(const OutputPointer& other) const {
            return txNum == other.txNum && outputNum == other.outputNum;
        }
        
        bool operator<(const OutputPointer& other) const {
            if (txNum != other.txNum) {
                return txNum < other.txNum;
            } else {
                return outputNum < other.outputNum;
            }
        }
        
        std::string toString() const;
        
        Transaction getTransaction(const ChainAccess &access) const;
        const Output &getOutput(const ChainAccess &access) const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction getTransaction() const;
        const Output &getOutput() const;
        #endif
    };
}

#endif /* output_pointer_hpp */
