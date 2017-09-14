//
//  address_traverser.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/29/17.
//
//

#ifndef address_traverser_hpp
#define address_traverser_hpp

#include "parser_index.hpp"

#include <string>
#include <cstdint>
#include <stdio.h>

namespace blocksci {
    struct OutputPointer;
    struct Address;
    struct Transaction;
    class ScriptAccess;
    class ScriptFirstSeenAccess;
    class ScriptAccess;
}


class AddressTraverser : public ParserIndex {
    
    void processP2SHAddress(const blocksci::ScriptAccess &access, const blocksci::Address &pointer, uint32_t txNum, uint32_t p2shNum);
    
    virtual void sawAddress(const blocksci::Address &pointer, uint32_t txNum) = 0;
    
    virtual void linkP2SHAddress(const blocksci::Address &pointer, uint32_t txNum, uint32_t p2shNum) = 0;
    
    void processTx(const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts, const blocksci::Transaction &tx) override;
    
public:
    AddressTraverser(const ParserConfiguration &config, const std::string &resultName);
};

#endif /* address_traverser_hpp */
