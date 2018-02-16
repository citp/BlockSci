//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef hash_index_hpp
#define hash_index_hpp

#include <blocksci/blocksci_fwd.hpp>

#include <rocksdb/db.h>

#include <array>
#include <vector>
#include <cstdint>


namespace blocksci {
    struct Address;
    
    class HashIndex {
    public:
        struct IndexType {
            enum Enum {
                PubkeyHash, ScriptHash, Tx
            };
            static constexpr size_t size = 3;
            static constexpr std::array<Enum, size> all = {{PubkeyHash, ScriptHash, Tx}};
            static constexpr std::array<const char*, 3> tableNames = {{"PUBKEYHASH_ADDRESS", "P2SH_ADDRESS", "TXHASH"}};
        };
        
        HashIndex(const std::string &path);
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeyhash);
        uint32_t getScriptHashIndex(const uint160 &scripthash);
        uint32_t getTxIndex(const uint256 &txHash);
    private:
        rocksdb::DB *db;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandles;
    };
}

#endif /* hash_index_hpp */
