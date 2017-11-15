//
//  hash_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#include "hash_index.hpp"
#include "data_configuration.hpp"
#include "bitcoin_uint256.hpp"
#include "scripts/bitcoin_base58.hpp"
#include "address/address.hpp"
#include "util.hpp"

#include <array>

namespace blocksci {
    
    lmdb::env createHashIndexEnviroment(const std::string &path) {
        auto env = lmdb::env::create();
        env.set_mapsize(100UL * 1024UL * 1024UL * 1024UL); /* 1 GiB */
        env.set_max_dbs(5);
        env.open(path.c_str(), MDB_NOSUBDIR, 0664);
        return env;
    }
    
    HashIndex::HashIndex(const std::string &path) : env(createHashIndexEnviroment(path)) {}
    
    template <typename T>
    uint32_t getMatch(const lmdb::env &env, const char *name, const T &t) {
        auto rtxn = lmdb::txn::begin(env, nullptr, MDB_RDONLY);
        auto dbi = lmdb::dbi::open(rtxn, name);
        auto cursor = lmdb::cursor::open(rtxn, dbi);
        lmdb::val key{&t, sizeof(t)};
        lmdb::val data;
        bool found = dbi.get(rtxn, key, data);
        if (found) {
            return *data.data<uint32_t>();
        } else {
            return 0;
        }
    }
    
    uint32_t HashIndex::getTxIndex(const uint256 &txHash) const {
        return getMatch(env, "tx", txHash);
    }
    
    uint32_t HashIndex::getPubkeyHashIndex(const uint160 &pubkeyhash) const {
        return getMatch(env, "pubkey", pubkeyhash);
        
    }
    
    uint32_t HashIndex::getScriptHashIndex(const uint160 &scripthash) const {
        return getMatch(env, "scripthash", scripthash);
    }
}
