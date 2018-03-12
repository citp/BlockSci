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
#include "progress_bar.hpp"

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/util/state.hpp>

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction_range.hpp>
#include <blocksci/scripts/script_access.hpp>

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
    void updateScript(std::true_type, EquivType type, const blocksci::State &state, const blocksci::DataAccess &access) {
        auto typeIndex = static_cast<size_t>(type);
        auto progress = makeProgressBar(state.scriptCounts[typeIndex] - latestState.scriptCounts[typeIndex], [=]() {});
        uint32_t num = 0;
        std::cout << "Updating index with scripts of type " << equivAddressName(type) << "\n";
        for (uint32_t i = latestState.scriptCounts[typeIndex]; i < state.scriptCounts[typeIndex]; i++) {
            static_cast<T*>(this)->template processScript<type>(i + 1, access);
            progress.update(num);
            num++;
        }
    }
    
    template<typename EquivType>
    void updateScript(std::false_type, EquivType, const blocksci::State &, const blocksci::DataAccess &) {}
    
    virtual void prepareUpdate() {}
    void runUpdate(const blocksci::State &state) {
        blocksci::DataAccess access(config);
        
        if (latestState.txCount < state.txCount) {
            auto newTransactions = blocksci::TransactionRange(access, latestState.txCount, state.txCount);
            auto newCount = ranges::distance(newTransactions);
            std::cout << "Updating index with " << newCount << " txes\n";
            auto progress = makeProgressBar(newCount, [=]() {});
            uint32_t num = 0;
            RANGES_FOR(auto tx, newTransactions) {
                static_cast<T*>(this)->processTx(tx);
                progress.update(num);
                num++;
            }
        }
        
        blocksci::for_each(blocksci::DedupAddressInfoList(), [&](auto type) {
            updateScript(ParserIndexScriptInfo<T, type>{}, type, state, access);
        });
        latestState = state;
    };
    
    virtual void tearDown() {}
};


#endif /* parser_index_hpp */
