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

ParserIndex::ParserIndex(const ParserConfiguration &config_, const std::string &resultName) : updateFuture{std::async(std::launch::async, [&] {})}, config(config_), cachePath(config_.parserDirectory()/(resultName + ".txt")) {
    boost::filesystem::ifstream inputFile(cachePath);
    inputFile >> latestTx;
}

void ParserIndex::preDestructor() {
    if (updateFuture.valid()) {
        updateFuture.get();
    }
}

ParserIndex::~ParserIndex() {
    // Must call this in subclasses to finish update before subclass destructor
    preDestructor();
    boost::filesystem::ofstream outputFile(cachePath);
    outputFile << latestTx;
}

void ParserIndex::update(const std::vector<uint32_t> &revealed) {
    using namespace std::chrono_literals;
    waitingRevealed.insert(waitingRevealed.end(), revealed.begin(), revealed.end());
    if (updateFuture.wait_for(0ms) == std::future_status::ready) {
        updateFuture.get();
        auto nextRevealed = std::move(waitingRevealed);
        waitingRevealed.clear();
        blocksci::ChainAccess chain{config, false, 0};
        auto maxTxCount = chain.txCount();
        blocksci::ScriptAccess scripts{config};
        prepareUpdate(chain, scripts);
        updateFuture = std::async(std::launch::async, [this, nextRevealed, maxTxCount] {
            runUpdate(nextRevealed, maxTxCount);
        });
    }
}

void ParserIndex::runUpdate(const std::vector<uint32_t> &revealed, uint32_t maxTxCount) {
    blocksci::ChainAccess chain{config, false, 0};
    blocksci::ScriptAccess scripts{config};
    
    auto currentCount = chain.txCount();
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
