//
//  parser_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/13/17.
//
//

#include "parser_index.hpp"
#include "block_processor.hpp"

#include <blocksci/util.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script_pointer.hpp>

#include <boost/filesystem/fstream.hpp>

using namespace blocksci;

ParserIndex::ParserIndex(const ParserConfiguration &config_, const std::string &resultName) : config(config_), cachePath(config_.parserDirectory()/(resultName + ".txt")) {
    if (boost::filesystem::exists(cachePath)) {
        boost::filesystem::ifstream inputFile(cachePath);
        inputFile >> latestState;
    }
}

ParserIndex::~ParserIndex() {
    boost::filesystem::ofstream outputFile(cachePath);
    outputFile << latestState;
}

void ParserIndex::runUpdate(const std::vector<uint32_t> &revealed, const State &state) {
    blocksci::ChainAccess chain{config, false, 0};
    blocksci::ScriptAccess scripts{config};
    
    auto currentCount = static_cast<uint32_t>(chain.txCount());
    if (latestState.txCount < currentCount) {
        auto newTransactions = iterateTransactions(chain, latestState.txCount, state.txCount);
        for (auto tx : newTransactions) {
            processTx(tx, chain, scripts);
        }
    }
    
    for_each(ScriptType::all, [&](auto type) {
        auto typeIndex = static_cast<size_t>(type);
        for (uint32_t i = latestState.scriptCounts[typeIndex]; i < state.scriptCounts[typeIndex]; i++) {
            auto pointer = ScriptPointer{i + 1, type};
            processScript(pointer, chain, scripts);
        }
    });
    
    for (auto reveal : revealed) {
        auto p2sh = blocksci::script::ScriptHash{scripts, reveal};
        revealedP2SH(p2sh, scripts);
    }
    
    latestState = state;
}
