//
//  parser_index_creator.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 9/22/17.
//

#include "parser_index_creator.hpp"
#include "parser_index.hpp"
#include <blocksci/state.hpp>

#include <blocksci/chain/chain_access.hpp>
#include <blocksci/scripts/script_access.hpp>

ParserIndexCreator::~ParserIndexCreator() {
    if (!tornDown) {
        blocksci::ChainAccess chain{config, false, 0};
        blocksci::ScriptAccess scripts{config};
        blocksci::State updateState{chain, scripts};
        complete(updateState);
    }
    teardownFuture.get();
}

void ParserIndexCreator::update(const std::vector<uint32_t> &revealed, blocksci::State state) {
    using namespace std::chrono_literals;
    
    waitingRevealed.insert(waitingRevealed.end(), revealed.begin(), revealed.end());
    if (updateFuture.wait_for(0ms) == std::future_status::ready) {
        updateFuture.get();
        launchingUpdate = true;
        auto nextRevealed = std::move(waitingRevealed);
        waitingRevealed.clear();
        index->prepareUpdate();
        updateFuture = std::async(std::launch::async, [this, nextRevealed, state] {
            index->runUpdate(nextRevealed, state);
            launchingUpdate = false;
        });
    }
}

void ParserIndexCreator::complete(blocksci::State state) {
    using namespace std::chrono_literals;
    assert(!tornDown);
    tornDown = true;
    teardownFuture = std::async(std::launch::async, [&, state] {
        while (launchingUpdate) {
            std::this_thread::sleep_for(100ms);
        }
        if (updateFuture.valid()) {
            updateFuture.get();
        }
        index->runUpdate(waitingRevealed, state);
        index->tearDown();
    });
}
