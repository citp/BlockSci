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

#include <blocksci/core/raw_transaction.hpp>

#include <internal/chain_access.hpp>
#include <internal/progress_bar.hpp>
#include <internal/script_access.hpp>
#include <internal/state.hpp>

#include <range/v3/range_for.hpp>

#include <wjfilesystem/path.h>

#include <iostream>
#include <fstream>

template <typename T, blocksci::DedupAddressType::Enum type>
struct ParserIndexScriptInfo;

template <typename T>
class ParserIndex {
protected:
    const ParserConfigurationBase &config;
    filesystem::path cachePath;
    blocksci::State latestState;
public:
    ParserIndex(const ParserConfigurationBase &config_, const std::string &resultName) : config(config_), cachePath(config_.parserDirectory()/(resultName + ".txt")) {
        if (cachePath.exists()) {
            std::ifstream inputFile(cachePath.str());
            inputFile >> latestState;
        }
    }
    ParserIndex(const ParserIndex &) = delete;
    ParserIndex &operator=(const ParserIndex &) = delete;
    ParserIndex(ParserIndex &&) = delete;
    ParserIndex &operator=(ParserIndex &&) = delete;
    ~ParserIndex() {
        std::ofstream outputFile(cachePath.str());
        outputFile << latestState;
    }
    
    template<typename EquivType>
    void updateScript(std::true_type, EquivType type, const blocksci::State &state, const blocksci::ScriptAccess &scripts) {
        auto typeIndex = static_cast<size_t>(type);
        auto progress = blocksci::makeProgressBar(state.scriptCounts[typeIndex] - latestState.scriptCounts[typeIndex], [=]() {});
        uint32_t num = 0;
        std::cout << "Updating index with scripts of type " << dedupAddressName(type) << "\n";
        for (uint32_t i = latestState.scriptCounts[typeIndex]; i < state.scriptCounts[typeIndex]; i++) {
            static_cast<T*>(this)->template processScript<type>(i + 1, scripts);
            progress.update(num);
            num++;
        }
    }
    
    template<typename EquivType>
    void updateScript(std::false_type, EquivType, const blocksci::State &, const blocksci::ScriptAccess &) {}
    
    void runUpdate(const blocksci::State &state);
};

template <typename T>
struct ParserScriptUpdater {
    ParserIndex<T> &index;
    const blocksci::State &state;
    const blocksci::ScriptAccess &scripts;
    
    ParserScriptUpdater(ParserIndex<T> &index_, const blocksci::State &state_, const blocksci::ScriptAccess &scripts_): index(index_), state(state_), scripts(scripts_) {}
    
    template <typename V>
    void operator()(V v) {
        index.updateScript(ParserIndexScriptInfo<T, V{}>{}, v, state, scripts);
    }
};

template <typename T>
void ParserIndex<T>::runUpdate(const blocksci::State &state) {
    blocksci::ChainAccess chain{config.dataConfig.chainDirectory(), config.dataConfig.blocksIgnored, config.dataConfig.errorOnReorg};
    blocksci::ScriptAccess scripts{config.dataConfig.scriptsDirectory()};
    
    if (latestState.txCount < state.txCount) {
        auto newCount = state.txCount - latestState.txCount;
        std::cout << "Updating index with " << newCount << " txes\n";
        auto progress = blocksci::makeProgressBar(newCount, [=]() {});
        uint32_t num = 0;
        for (uint32_t txNum = latestState.txCount; txNum < state.txCount; txNum++) {
            auto tx = chain.getTx(txNum);
            static_cast<T*>(this)->processTx(tx, txNum, chain, scripts);
            progress.update(num);
            num++;
        }
    }
        
    ParserScriptUpdater<T> updater(*this, state, scripts);
    blocksci::for_each(blocksci::DedupAddressType::all(), updater);
    latestState = state;
}

#endif /* parser_index_hpp */
