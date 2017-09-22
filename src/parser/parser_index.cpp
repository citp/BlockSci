//
//  parser_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/13/17.
//
//

#include "parser_index.hpp"
#include "block_processor.hpp"

#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#include <boost/filesystem/fstream.hpp>

ParserIndex::ParserIndex(const ParserConfiguration &config_, const std::string &resultName) : config(config_), cachePath(config_.parserDirectory()/(resultName + ".txt")) {
    if (boost::filesystem::exists(cachePath)) {
        boost::filesystem::ifstream inputFile(cachePath);
        inputFile >> latestTx;
    } else {
        latestTx = 0;
    }
}

ParserIndex::~ParserIndex() {
    boost::filesystem::ofstream outputFile(cachePath);
    outputFile << latestTx;
}

void ParserIndex::runUpdate(const std::vector<uint32_t> &revealed, uint32_t maxTxCount) {
    blocksci::ChainAccess chain{config, false, 0};
    blocksci::ScriptAccess scripts{config};
    
    auto currentCount = static_cast<uint32_t>(chain.txCount());
    if (latestTx < currentCount) {
        auto newTransactions = iterateTransactions(chain, latestTx, maxTxCount);
        for (auto tx : newTransactions) {
            processTx(chain, scripts, tx);
        }
    }
    
    for (auto reveal : revealed) {
        auto p2sh = blocksci::script::ScriptHash{scripts, reveal};
        revealedP2SH(p2sh, scripts);
    }
    
    latestTx = currentCount;
}
