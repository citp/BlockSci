//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#include "hash_index_creator.hpp"
#include "parser_configuration.hpp"
#include "raw_address_visitor.hpp"

#include <blocksci/core/raw_address.hpp>

HashIndexCreator::HashIndexCreator(const ParserConfigurationBase &config_, const filesystem::path &path) : ParserIndex(config_, "hashIndex"), db(path, false) {}

template <bool, blocksci::AddressType::Enum type>
struct ClearerFunctor;

template <blocksci::AddressType::Enum type>
struct ClearerFunctor<true, type> {
    HashIndexAddressCache<type> &cache;
    blocksci::HashIndex &db;
    
    ClearerFunctor(HashIndexAddressCache<type> &cache_, blocksci::HashIndex &db_) : cache(cache_), db(db_) {}
    
    void operator()() {
        std::vector<std::pair<typename blocksci::AddressInfo<type>::IDType, uint32_t>> rows;
        rows.reserve(cache.size());
        for (const auto &pair : cache) {
            rows.emplace_back(pair.first.key, pair.second);
        }
        cache.clear();
        db.addAddresses<type>(std::move(rows));
    }
};

template <blocksci::AddressType::Enum type>
struct ClearerFunctor<false, type> {
    ClearerFunctor(HashIndexAddressCache<type> &, blocksci::HashIndex &) {}
    void operator()() {
        
    }
};

HashIndexCreator::~HashIndexCreator() {
    clearTxCache();
    // Duplicated to avoid crash in GCC 7.2
    for_each(blocksci::AddressType::all{}, [&](auto tag) {
        ClearerFunctor<!std::is_same<typename blocksci::AddressInfo<tag.value>::IDType, void>::value, tag.value>{std::get<HashIndexAddressCache<tag.value>>(addressCache), db}();
        
    });
}

void HashIndexCreator::processTx(const blocksci::RawTransaction *tx, uint32_t txNum, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) {
    addTx(*chain.getTxHash(txNum), txNum);
    bool insideP2SH;
    std::function<bool(const blocksci::RawAddress &)> inputVisitFunc = [&](const blocksci::RawAddress &a) {
        if (a.type == blocksci::AddressType::SCRIPTHASH) {
            insideP2SH = true;
            return true;
        } else if (a.type == blocksci::AddressType::WITNESS_SCRIPTHASH && insideP2SH) {
            auto script = scripts.getScriptData<blocksci::DedupAddressType::SCRIPTHASH>(a.scriptNum);
            addAddress<blocksci::AddressType::WITNESS_SCRIPTHASH>(script->hash256, a.scriptNum);
            return false;
        } else {
            return false;
        }
    };
    auto inputs = ranges::make_subrange(tx->beginInputs(), tx->endInputs());
    for (auto input : inputs) {
        insideP2SH = false;
        visit(blocksci::RawAddress{input.getAddressNum(), input.getType()}, inputVisitFunc, scripts);
    }
    
    auto outputs = ranges::make_subrange(tx->beginOutputs(), tx->endOutputs());
    for (auto &txout : outputs) {
        if (txout.getType() == blocksci::AddressType::WITNESS_SCRIPTHASH) {
            auto script = scripts.getScriptData<blocksci::DedupAddressType::SCRIPTHASH>(txout.getAddressNum());
            addAddress<blocksci::AddressType::WITNESS_SCRIPTHASH>(script->hash256, txout.getAddressNum());
        }
    }
}

void HashIndexCreator::addTx(const blocksci::uint256 &hash, uint32_t txNum) {
    txCache.insert(hash, txNum);
    if (txCache.isFull()) {
        clearTxCache();
    }
}

ranges::optional<uint32_t> HashIndexCreator::getTxIndex(const blocksci::uint256 &txHash) {
    auto it = txCache.find(txHash);
    if (it != txCache.end()) {
        return it->second;
    } else {
        return db.getTxIndex(txHash);
    }
}

void HashIndexCreator::clearTxCache() {
    std::vector<std::pair<blocksci::uint256, uint32_t>> rows;
    rows.reserve(txCache.size());
    for (const auto &pair : txCache) {
        rows.emplace_back(pair.first.key, pair.second);
    }
    txCache.clear();
    db.addTxes(std::move(rows));
}
