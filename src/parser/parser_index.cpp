//
//  parser_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/13/17.
//
//

#include "parser_index.hpp"

#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/scripts/script_access.hpp>

ParserIndex::ParserIndex(const ParserConfiguration &config_, const std::string &resultName) : config(config_), cachePath(config_.parserDirectory()/(resultName + ".txt")) {
    boost::filesystem::ifstream inputFile(cachePath);
    inputFile >> latestTx;
}

ParserIndex::~ParserIndex() {
    boost::filesystem::ofstream outputFile(cachePath);
    outputFile << latestTx;
    
}

void ParserIndex::update() {
    blocksci::ChainAccess chain{config, false, 0};
    blocksci::ScriptAccess scripts{config};
    
    prepareUpdate(chain, scripts);
    auto currentCount = chain.txCount();
    if (latestTx < currentCount) {
        auto newTransactions = iterateTransactions(chain, latestTx, currentCount);
        for (auto tx : newTransactions) {
            processTx(chain, scripts, tx);
        }
    }
    
    latestTx = currentCount;
}
