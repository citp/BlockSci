//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#include "hash_index_creator.hpp"
#include "parser_configuration.hpp"

#include <blocksci/util.hpp>
#include <blocksci/hash_index.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#include <sstream>

HashIndexCreator::HashIndexCreator(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "hashIndex"), env(blocksci::createHashIndexEnviroment(path)), wtxn(lmdb::txn::begin(env)), pubkey_dbi(lmdb::dbi::open(wtxn, "pubkey", MDB_CREATE)), scripthash_dbi(lmdb::dbi::open(wtxn, "scripthash", MDB_CREATE)), tx_dbi(lmdb::dbi::open(wtxn, "tx", MDB_CREATE)) {
}

void HashIndexCreator::tearDown() {
    wtxn.commit();
}

void HashIndexCreator::processTx(const blocksci::Transaction &tx, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &) {
    auto hash = tx.getHash(chain);
    tx_dbi.put(wtxn, hash, tx.txNum, lmdb::dbi::default_put_flags);
}

void HashIndexCreator::processScript(const blocksci::Script &script, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    if (script.type == blocksci::ScriptType::Enum::PUBKEY) {
        blocksci::script::Pubkey pubkeyScript{scripts, script.scriptNum};
        pubkey_dbi.put(wtxn, pubkeyScript.pubkeyhash, script.scriptNum, lmdb::dbi::default_put_flags);
    } else if (script.type == blocksci::ScriptType::Enum::SCRIPTHASH) {
        blocksci::script::ScriptHash p2shScript{scripts, script.scriptNum};
        scripthash_dbi.put(wtxn, p2shScript.address, script.scriptNum, lmdb::dbi::default_put_flags);
    }
}

void HashIndexCreator::rollback(const blocksci::State &state) {
    {
        // Delete pubkeys
        auto cursor = lmdb::cursor::open(wtxn, pubkey_dbi);
        lmdb::val key, value;
        while (cursor.get(key, value, MDB_NEXT)) {
            if (*value.data<uint32_t>() >= state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::PUBKEY)]) {
                lmdb::cursor_del(cursor.handle());
            }
        }
    }
    
    {
        // Delete scripthashes
        auto cursor = lmdb::cursor::open(wtxn, scripthash_dbi);
        lmdb::val key, value;
        while (cursor.get(key, value, MDB_NEXT)) {
            if (*value.data<uint32_t>() >= state.scriptCounts[static_cast<size_t>(blocksci::ScriptType::Enum::SCRIPTHASH)]) {
                lmdb::cursor_del(cursor.handle());
            }
        }
    }
    
    {
        // Delete txes
        auto cursor = lmdb::cursor::open(wtxn, tx_dbi);
        lmdb::val key, value;
        while (cursor.get(key, value, MDB_NEXT)) {
            if (*value.data<uint32_t>() >= state.txCount) {
                lmdb::cursor_del(cursor.handle());
            }
        }
    }
}
