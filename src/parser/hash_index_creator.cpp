//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#include "hash_index_creator.hpp"
#include "parser_configuration.hpp"

#include <blocksci/util/util.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#include <sstream>

HashIndexCreator::HashIndexCreator(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "hashIndex"), db(path, false) {}

HashIndexCreator::~HashIndexCreator() {
    clearTxCache();
    for_each(blocksci::AddressInfoList{}, [&](auto tag) {
        clearAddressCache<tag>();
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
            auto script = scripts.getScriptData<blocksci::AddressType::WITNESS_SCRIPTHASH>(a.scriptNum);
            addAddress<blocksci::AddressType::WITNESS_SCRIPTHASH>(script->hash256, a.scriptNum);
            return false;
        } else {
            return false;
        }
    };
    for (auto input : tx->inputs()) {
        insideP2SH = false;
        visit(blocksci::RawAddress{input.toAddressNum, input.getType()}, inputVisitFunc, scripts);
    }
    
    for (auto &txout : tx->outputs()) {
        if (txout.getType() == blocksci::AddressType::WITNESS_SCRIPTHASH) {
            auto script = scripts.getScriptData<blocksci::AddressType::WITNESS_SCRIPTHASH>(txout.toAddressNum);
            addAddress<blocksci::AddressType::WITNESS_SCRIPTHASH>(script->hash256, txout.toAddressNum);
        }
    }
}

void HashIndexCreator::addTx(const blocksci::uint256 &hash, uint32_t txNum) {
    txCache.insert(hash, txNum);
    if (txCache.isFull()) {
        clearTxCache();
    }
}

uint32_t HashIndexCreator::getTxIndex(const blocksci::uint256 &txHash) {
    auto it = txCache.find(txHash);
    if (it != txCache.end()) {
        return it->second;
    } else {
        return db.getTxIndex(txHash);
    }
}

void HashIndexCreator::clearTxCache() {
    rocksdb::WriteBatch batch;
    for (const auto &pair : txCache) {
        rocksdb::Slice keySlice(reinterpret_cast<const char *>(&pair.first), sizeof(pair.first));
        rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&pair.second), sizeof(pair.second));
        batch.Put(db.getTxColumn().get(), keySlice, valueSlice);
    }
    db.writeBatch(batch);
    txCache.clear();
}

void HashIndexCreator::rollback(const blocksci::State &state) {
    {
        auto &column = db.getColumn(blocksci::AddressType::WITNESS_SCRIPTHASH);
        rocksdb::WriteBatch batch;
        auto it = db.getIterator(blocksci::AddressType::WITNESS_SCRIPTHASH);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t destNum;
            memcpy(&destNum, it->value().data(), sizeof(destNum));
            auto count = state.scriptCounts[static_cast<size_t>(blocksci::DedupAddressType::SCRIPTHASH)];
            if (destNum >= count) {
                batch.Delete(column.get(), it->key());
            }
        }
        assert(it->status().ok());
        db.writeBatch(batch);
    }
    
    {
        auto it = db.getTxIterator();
        rocksdb::WriteBatch batch;
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t value;
            memcpy(&value, it->value().data(), sizeof(value));
            if (value >= state.txCount) {
                batch.Delete(db.getTxColumn().get(), it->key());
            }
        }
        db.writeBatch(batch);
        assert(it->status().ok()); // Check for any errors found during the scan
    }
}
