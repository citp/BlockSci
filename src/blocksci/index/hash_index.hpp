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

#include <lmdbxx/lmdb++.h>

#include <array>
#include <vector>
#include <cstdint>


namespace blocksci {
    struct Address;
    
    lmdb::env createHashIndexEnviroment(const std::string &path);
    
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
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeyhash) const;
        uint32_t getScriptHashIndex(const uint160 &scripthash) const;
        uint32_t getTxIndex(const uint256 &txHash) const;
    private:
        lmdb::env env;
    };
}

#endif /* hash_index_hpp */
