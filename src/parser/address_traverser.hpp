//
//  address_traverser.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/29/17.
//
//

#ifndef address_traverser_hpp
#define address_traverser_hpp

namespace blocksci {
    struct OutputPointer;
    struct Address;
    struct Transaction;
    class ScriptAccess;
    class AddressFirstSeenAccess;
    class DataAccess;
}

#include <cstdint>
#include <stdio.h>

class AddressTraverser {
    
    void processP2SHAddress(const blocksci::ScriptAccess &access, const blocksci::Address &pointer, uint32_t txNum, uint32_t p2shNum);
    
    virtual void sawAddress(const blocksci::Address &pointer, uint32_t txNum) = 0;
    
    virtual void linkP2SHAddress(const blocksci::Address &pointer, uint32_t txNum, uint32_t p2shNum) = 0;
    
public:
    void processTx(const blocksci::DataAccess &access, const blocksci::Transaction &tx);
};

#endif /* address_traverser_hpp */
