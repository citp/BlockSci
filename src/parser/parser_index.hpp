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

#include <boost/filesystem/path.hpp>

#include <future>
#include <stdio.h>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    struct Transaction;
    struct Address;
}

struct RevealedScriptHash;

class ParserIndex {
private:
    std::future<void> updateFuture;
protected:
    ParserConfiguration config;
    boost::filesystem::path cachePath;
    uint32_t latestTx;
    std::vector<uint32_t> waitingRevealed;
    
    virtual void processTx(const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts, const blocksci::Transaction &tx) = 0;
    virtual void revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) = 0;
    virtual void prepareUpdate(const blocksci::ChainAccess &, const blocksci::ScriptAccess &) {}
    
    void runUpdate(const std::vector<uint32_t> &revealed, uint32_t maxTxCount);
    
public:
    ParserIndex(const ParserConfiguration &config, const std::string &resultName);
    virtual ~ParserIndex();
    
    void update(const std::vector<uint32_t> &revealed);
};

#endif /* parser_index_hpp */
