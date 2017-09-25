//
//  parser_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/13/17.
//
//

#ifndef parser_index_hpp
#define parser_index_hpp

#include "parser_configuration.hpp"

#include <blocksci/scripts/scriptsfwd.hpp>
#include <blocksci/state.hpp>

#include <boost/filesystem/path.hpp>

#include <future>
#include <stdio.h>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    struct Transaction;
    struct Address;
    struct State;
    struct ScriptPointer;
}

class ParserIndex {
protected:
    ParserConfiguration config;
    boost::filesystem::path cachePath;
    blocksci::State latestState;
    
    virtual void processTx(const blocksci::Transaction &tx, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) = 0;
    virtual void processScript(const blocksci::ScriptPointer &pointer, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) = 0;
    virtual void revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) = 0;
    
public:
    ParserIndex(const ParserConfiguration &config, const std::string &resultName);
    virtual ~ParserIndex();
    
    virtual void prepareUpdate() {}
    void runUpdate(const std::vector<uint32_t> &revealed, const blocksci::State &state);
    virtual void tearDown() {}
    void preDestroy();
};

#endif /* parser_index_hpp */
