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

void HashIndexCreator::processTx(const blocksci::Transaction &tx, const blocksci::ScriptAccess &scripts) {
    auto hash = tx.getHash();
    db.addTx(hash, tx.txNum);
    
    for (auto txout : tx.outputs()) {
        if (txout.getType() == blocksci::AddressType::WITNESS_SCRIPTHASH) {
            auto scriptNum = txout.getAddress().scriptNum;
            auto script = blocksci::script::WitnessScriptHash(scripts, scriptNum);
            db.addAddress<blocksci::AddressType::WITNESS_SCRIPTHASH>(script.address, scriptNum);
        }
    }
}

//void HashIndexCreator::processScript(const blocksci::Script &script, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
//    if (script.type == blocksci::ScriptType::Enum::PUBKEY) {
//        blocksci::script::Pubkey pubkeyScript{scripts, script.scriptNum};
//        rocksdb::Slice keySlice(reinterpret_cast<const char *>(&pubkeyScript.pubkeyhash), sizeof(pubkeyScript.pubkeyhash));
//        rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&script.scriptNum), sizeof(script.scriptNum));
//        db->Put(rocksdb::WriteOptions(), columnHandles[pubkeyHandleNum], keySlice, valueSlice);
//    } else if (script.type == blocksci::ScriptType::Enum::SCRIPTHASH) {
//        blocksci::script::ScriptHash p2shScript{scripts, script.scriptNum};
//        rocksdb::Slice keySlice(reinterpret_cast<const char *>(&p2shScript.address), sizeof(p2shScript.address));
//        rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&script.scriptNum), sizeof(script.scriptNum));
//        db->Put(rocksdb::WriteOptions(), columnHandles[scriptHashHandleNum], keySlice, valueSlice);
//    }
//}

void HashIndexCreator::rollback(const blocksci::State &state) {
//    {
//        auto column = columnHandles[pubkeyHandleNum];
//        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
//        for (it->SeekToFirst(); it->Valid(); it->Next()) {
//            uint32_t value;
//            memcpy(&value, it->value().data(), sizeof(value));
//            if (value >= state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::PUBKEY)]) {
//                db->Delete(rocksdb::WriteOptions(), column, it->key());
//            }
//        }
//        assert(it->status().ok()); // Check for any errors found during the scan
//        delete it;
//    }
//
//    {
//        auto column = columnHandles[scriptHashHandleNum];
//        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
//        for (it->SeekToFirst(); it->Valid(); it->Next()) {
//            uint32_t value;
//            memcpy(&value, it->value().data(), sizeof(value));
//            if (value >= state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::SCRIPTHASH)]) {
//                db->Delete(rocksdb::WriteOptions(), column, it->key());
//            }
//        }
//        assert(it->status().ok()); // Check for any errors found during the scan
//        delete it;
//    }
    
    {
        auto column = db.getColumn(blocksci::AddressType::WITNESS_SCRIPTHASH);
        rocksdb::WriteBatch batch;
        auto it = db.getIterator(blocksci::AddressType::WITNESS_SCRIPTHASH);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t destNum;
            memcpy(&destNum, it->value().data(), sizeof(destNum));
            auto count = state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::SCRIPTHASH)];
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
