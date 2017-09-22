//
//  parser_index_creator.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 9/22/17.
//

#include "parser_index_creator.hpp"
#include "parser_index.hpp"

#include <blocksci/chain/chain_access.hpp>

ParserIndexCreator::~ParserIndexCreator() {
    blocksci::ChainAccess chain{config, false, 0};
    auto maxTxCount = static_cast<uint32_t>(chain.txCount());
    complete(maxTxCount);
    teardownFuture.get();
}

void ParserIndexCreator::update(const std::vector<uint32_t> &revealed, uint32_t maxTxCount) {
    using namespace std::chrono_literals;
    
    waitingRevealed.insert(waitingRevealed.end(), revealed.begin(), revealed.end());
    if (updateFuture.wait_for(0ms) == std::future_status::ready) {
        updateFuture.get();
        launchingUpdate = true;
        auto nextRevealed = std::move(waitingRevealed);
        waitingRevealed.clear();
        index->prepareUpdate();
        updateFuture = std::async(std::launch::async, [this, nextRevealed, maxTxCount] {
            index->runUpdate(nextRevealed, maxTxCount);
            launchingUpdate = false;
        });
    }
}

void ParserIndexCreator::complete(uint32_t maxTxCount) {
    using namespace std::chrono_literals;
    if (!tornDown) {
        tornDown = true;
        teardownFuture = std::async(std::launch::async, [&] {
            while (launchingUpdate) {
                std::this_thread::sleep_for(100ms);
            }
            if (updateFuture.valid()) {
                updateFuture.get();
            }
            index->runUpdate(waitingRevealed, maxTxCount);
            index->tearDown();
        });
    }
}
