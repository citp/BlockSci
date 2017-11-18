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

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/util/state.hpp>

#include <future>

namespace blocksci {
    struct Address;
}

class ParserIndex {
protected:
    const ParserConfigurationBase &config;
    boost::filesystem::path cachePath;
    blocksci::State latestState;
    
    virtual void processTx(const blocksci::Transaction &tx, const blocksci::ScriptAccess &scripts) = 0;
    virtual void processScript(const blocksci::Script &script, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) = 0;
    
public:
    ParserIndex(const ParserConfigurationBase &config, const std::string &resultName);
    ParserIndex(const ParserIndex &) = delete;
    ParserIndex &operator=(const ParserIndex &) = delete;
    ParserIndex(ParserIndex &&) = delete;
    ParserIndex &operator=(ParserIndex &&) = delete;
    virtual ~ParserIndex();
    
    virtual void prepareUpdate() {}
    void runUpdate(const blocksci::State &state);
    virtual void tearDown() {}
    void preDestroy();
    virtual void rollback(const blocksci::State &state) = 0;
};

#endif /* parser_index_hpp */
