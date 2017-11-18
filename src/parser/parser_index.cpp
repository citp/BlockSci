//
//  parser_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/13/17.
//
//

#include "parser_index.hpp"
#include "block_processor.hpp"

#include <blocksci/util/util.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction_range.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_info.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

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
    blocksci::ChainAccess chain{config, false, 0};
    blocksci::ScriptAccess scripts{config};
    
    if (latestState.txCount < state.txCount) {
        auto newTransactions = TransactionRange(chain, latestState.txCount, state.txCount);
        for (auto tx : newTransactions) {
            processTx(tx, scripts);
        }
    }
    
    for_each(ScriptType::all, [&](auto type) {
        auto typeIndex = static_cast<size_t>(type);
        for (uint32_t i = latestState.scriptCounts[typeIndex]; i < state.scriptCounts[typeIndex]; i++) {
            auto pointer = Script{i + 1, type};
            processScript(pointer, chain, scripts);
        }
    });
    
    latestState = state;
}
