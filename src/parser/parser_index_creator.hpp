//
//  parser_index_creator.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 9/22/17.
//

#ifndef parser_index_creator_hpp
#define parser_index_creator_hpp

#include "parser_configuration.hpp"

#include <future>
#include <vector>
#include <stdio.h>

namespace blocksci {
    class ChainAccess;
    class ScriptAccess;
    struct Transaction;
    struct Address;
}

class ParserIndex;

class ParserIndexCreator {
private:
    ParserConfiguration config;
    std::future<void> updateFuture;
    std::future<void> teardownFuture;
    std::atomic<bool> launchingUpdate;
    std::atomic<bool> tornDown;
    std::vector<uint32_t> waitingRevealed;
    
    std::unique_ptr<ParserIndex> index;
    
public:
    ParserIndexCreator(const ParserConfiguration &config_, std::unique_ptr<ParserIndex> index_) : config(config_), updateFuture{std::async(std::launch::async, [&] {})}, launchingUpdate(false), tornDown(false), index(std::move(index_)) {}
    ~ParserIndexCreator();
    
    void update(const std::vector<uint32_t> &revealed, uint32_t maxTxCount);
    void complete(uint32_t maxTxCount);
    
    template <typename IndexType>
    static ParserIndexCreator createIndex(const ParserConfiguration &config) {
        return ParserIndexCreator{config, std::make_unique<IndexType>(config)};
    }
};

#endif /* parser_index_creator_hpp */
