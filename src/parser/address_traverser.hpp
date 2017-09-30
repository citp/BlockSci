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

namespace blocksci {
    struct OutputPointer;
    struct Address;
    struct Transaction;
    class ScriptAccess;
    class ScriptFirstSeenAccess;
    class ScriptAccess;
}


class AddressTraverser : public ParserIndex {
    
    virtual void sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &outputPointer) = 0;
    
    void processTx(const blocksci::Transaction &tx, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) override;
    
    void processScript(const blocksci::ScriptPointer &, const blocksci::ChainAccess &, const blocksci::ScriptAccess &) override {}
    
public:
    AddressTraverser(const ParserConfigurationBase &config, const std::string &resultName);
};

#endif /* address_traverser_hpp */
