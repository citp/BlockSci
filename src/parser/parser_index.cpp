//
//  parser_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/13/17.
//
//

#include "parser_index.hpp"
#include "block_processor.hpp"
#include "progress_bar.hpp"

#include <blocksci/util/util.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction_range.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_info.hpp>

#include <range/v3/range_for.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <iostream>

using namespace blocksci;

ParserIndex::ParserIndex(const ParserConfigurationBase &config_, const std::string &resultName) : config(config_), cachePath(config_.parserDirectory()/(resultName + ".txt")) {
    if (boost::filesystem::exists(cachePath)) {
        boost::filesystem::ifstream inputFile(cachePath);
        inputFile >> latestState;
    }
}

ParserIndex::~ParserIndex() {
    boost::filesystem::ofstream outputFile(cachePath);
    outputFile << latestState;
}

void ParserIndex::runUpdate(const State &state) {
    blocksci::ChainAccess chain{config, false, blocksci::BlockHeight{0}};
    blocksci::ScriptAccess scripts{config};
    
    if (latestState.txCount < state.txCount) {
        auto newTransactions = TransactionRange(chain, latestState.txCount, state.txCount);
        auto newCount = ranges::distance(newTransactions);
        std::cout << "Updating index with " << newCount << " txes\n";
        auto progress = makeProgressBar(newCount, [=]() {});
        uint32_t num = 0;
        RANGES_FOR(auto tx, newTransactions) {
            processTx(tx, scripts);
            progress.update(num);
            num++;
        }
    }
    
    latestState = state;
}
