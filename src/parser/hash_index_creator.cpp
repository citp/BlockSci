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

void HashIndexCreator::processTx(const blocksci::Transaction &tx) {
    auto hash = tx.getHash();
    db.addTx(hash, tx.txNum);
    
    for (auto txout : tx.outputs()) {
        if (txout.getType() == blocksci::AddressType::WITNESS_SCRIPTHASH) {
            auto scriptNum = txout.getAddress().scriptNum;
            auto script = blocksci::script::WitnessScriptHash(scriptNum, tx.getAccess());
            db.addAddress<blocksci::AddressType::WITNESS_SCRIPTHASH>(script.address, scriptNum);
        }
    }
}

void HashIndexCreator::rollback(const blocksci::State &state) {
    {
        auto column = db.getColumn(blocksci::AddressType::WITNESS_SCRIPTHASH);
        rocksdb::WriteBatch batch;
        auto it = db.getIterator(blocksci::AddressType::WITNESS_SCRIPTHASH);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t destNum;
            memcpy(&destNum, it->value().data(), sizeof(destNum));
            auto count = state.scriptCounts[static_cast<size_t>(blocksci::EquivAddressType::SCRIPTHASH)];
            if (destNum >= count) {
                batch.Delete(column, it->key());
            }
        }
        assert(it->status().ok());
        delete it;
        db.writeBatch(batch);
    }
    
    {
        auto it = db.getTxIterator();
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t value;
            memcpy(&value, it->value().data(), sizeof(value));
            if (value >= state.txCount) {
                db.deleteTx(it->key());
            }
        }
        assert(it->status().ok()); // Check for any errors found during the scan
        delete it;
    }
}
