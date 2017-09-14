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

#include <boost/filesystem/path.hpp>

#include <stdio.h>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    struct Transaction;
}

class ParserIndex {
protected:
    ParserConfiguration config;
    boost::filesystem::path cachePath;
    uint32_t latestTx;
    
    virtual void processTx(const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts, const blocksci::Transaction &tx) = 0;
    virtual void prepareUpdate(const blocksci::ChainAccess &, const blocksci::ScriptAccess &) {}
    
public:
    ParserIndex(const ParserConfiguration &config, const std::string &resultName);
    virtual ~ParserIndex();
    
    
    
    void update();
};

#endif /* parser_index_hpp */
