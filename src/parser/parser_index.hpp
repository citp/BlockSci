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
#include "block_processor.hpp"

#include <blocksci/address/dedup_address.hpp>
#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction_range.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/util/progress_bar.hpp>
#include <blocksci/util/state.hpp>

#include <range/v3/range_for.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <iostream>
#include <future>

template <typename T, blocksci::DedupAddressType::Enum type>
struct ParserIndexScriptInfo;

template <typename T>
class ParserIndex {
protected:
    const ParserConfigurationBase &config;
    boost::filesystem::path cachePath;
    blocksci::State latestState;
public:
    ParserIndex(const ParserConfigurationBase &config_, const std::string &resultName) : config(config_), cachePath(config_.parserDirectory()/(resultName + ".txt")) {
        if (boost::filesystem::exists(cachePath)) {
            boost::filesystem::ifstream inputFile(cachePath);
            inputFile >> latestState;
        }
    }
    ParserIndex(const ParserIndex &) = delete;
    ParserIndex &operator=(const ParserIndex &) = delete;
    ParserIndex(ParserIndex &&) = delete;
    ParserIndex &operator=(ParserIndex &&) = delete;
    ~ParserIndex() {
        boost::filesystem::ofstream outputFile(cachePath);
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
    
    virtual void prepareUpdate() {}
    
    void runUpdate(const blocksci::State &state);
    
    virtual void tearDown() {}
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
    blocksci::ChainAccess chain{config.dataConfig};
    blocksci::ScriptAccess scripts{config.dataConfig};
    
    if (latestState.txCount < state.txCount) {
        auto newTransactions = blocksci::RawTransactionRange(chain, latestState.txCount, state.txCount);
        auto newCount = ranges::distance(newTransactions);
        std::cout << "Updating index with " << newCount << " txes\n";
        auto progress = blocksci::makeProgressBar(newCount, [=]() {});
        uint32_t num = 0;
        RANGES_FOR(auto tx, newTransactions) {
            static_cast<T*>(this)->processTx(tx.first, tx.second, chain, scripts);
            progress.update(num);
            num++;
        }
    }
        
    ParserScriptUpdater<T> updater(*this, state, scripts);
    blocksci::for_each(blocksci::DedupAddressType::all(), updater);
    latestState = state;
}

#endif /* parser_index_hpp */
