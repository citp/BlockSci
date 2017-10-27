//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef hash_index_hpp
#define hash_index_hpp

#include <lmdbxx/lmdb++.h>

#include <boost/filesystem/path.hpp>

#include <string_view>
#include <array>
#include <vector>
#include <cstdint>

struct sqlite3;
struct sqlite3_stmt;

namespace blocksci {
    using namespace std::string_view_literals;
    
    struct DataConfiguration;
    class uint256;
    class uint160;
    struct Address;
    
    lmdb::env createHashIndexEnviroment(const boost::filesystem::path &path);
    
    class HashIndex {
    public:
        struct IndexType {
            enum Enum {
                PubkeyHash, ScriptHash, Tx
            };
            static constexpr size_t size = 3;
            static constexpr std::array<Enum, size> all = {{PubkeyHash, ScriptHash, Tx}};
            static constexpr std::array<std::string_view, 3> tableNames = {{"PUBKEYHASH_ADDRESS"sv, "P2SH_ADDRESS"sv, "TXHASH"sv}};
        };
        
        HashIndex(const DataConfiguration &config);
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeyhash) const;
        uint32_t getScriptHashIndex(const uint160 &scripthash) const;
        uint32_t getTxIndex(const uint256 &txHash) const;
    private:
        lmdb::env env;
    };
}

#endif /* hash_index_hpp */
