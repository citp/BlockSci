//
//  hash_index_creator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/26/17.
//
//

#ifndef hash_index_creator_hpp
#define hash_index_creator_hpp

#include "parser_index.hpp"
#include "parser_fwd.hpp"
#include "serializable_map.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/util/hash.hpp>

#include <tuple>

namespace blocksci {
    class uint256;
}

class HashIndexCreator;

template<blocksci::DedupAddressType::Enum type>
struct ParserIndexScriptInfo<HashIndexCreator, type> : std::false_type {};

template <typename T, blocksci::AddressType::Enum type>
struct HashIndexAddressCacheImpl;

template <typename Key>
struct DenseHashMapWrappedKey {
    Key key;
    uint8_t extra;
    
    bool operator==(const DenseHashMapWrappedKey &other) const {
        return key == other.key && extra == other.extra;
    }
};

namespace std {
    template <typename T>
    struct hash<DenseHashMapWrappedKey<T>> {
        size_t operator()(const DenseHashMapWrappedKey<T> &item) const {
            std::size_t seed = 71854362;
            hash_combine(seed, item.key);
            hash_combine(seed, item.extra);
            return seed;
        }
    };
} // namespace std

template <typename Key>
struct DenseHashMapCache {
    using Cache = SerializableMap<DenseHashMapWrappedKey<Key>, uint32_t>;
    static constexpr int cacheSize = 20000;
    int cacheItems = 0;
    
    DenseHashMapCache() : cache({Key{}, 1}, {Key{}, 2}) {
        cache.resize(cacheSize);
    }
    
    void clear() {
        cache.clear_no_resize();
        cacheItems = 0;
    }
    
    bool isFull() const {
        return cacheItems == cacheSize;
    }
    
    void insert(const Key &id, uint32_t num) {
        cache.add(DenseHashMapWrappedKey<Key>{id, 0}, num);
        cacheItems++;
    }
    
    typename Cache::const_iterator find(const Key &id) const {
        return cache.find({id, 0});
    }
    
    typename Cache::const_iterator end() const {
        return cache.end();
    }
    
    auto begin() {
        return cache.begin();
    }
    
    auto end() {
        return cache.end();
    }
    
private:
    Cache cache;
};

template <blocksci::AddressType::Enum type>
struct HashIndexAddressCacheImpl<std::true_type, type> : public DenseHashMapCache<typename blocksci::AddressInfo<type>::IDType> {};

template <blocksci::AddressType::Enum type>
struct HashIndexAddressCacheImpl<std::false_type, type> {
    void clear() {}
    
    std::pair<DenseHashMapWrappedKey<int>, int> *begin() {
        return nullptr;
    }
    
    std::pair<DenseHashMapWrappedKey<int>, int> *end() {
        return nullptr;
    }
};

template <blocksci::AddressType::Enum type>
struct HashIndexAddressCache : public HashIndexAddressCacheImpl<std::integral_constant<bool, !std::is_same<typename blocksci::AddressInfo<type>::IDType, void>::value>, type> {};

class HashIndexCreator : public ParserIndex<HashIndexCreator> {
    blocksci::HashIndex db;
    
    using AddressCacheTuple = blocksci::to_address_tuple_t<HashIndexAddressCache>;
    
    DenseHashMapCache<blocksci::uint256> txCache;
    AddressCacheTuple addressCache;
    
    void clearTxCache();
    
    template<blocksci::AddressType::Enum type>
    void clearAddressCache() {
        auto &cache = std::get<HashIndexAddressCache<type>>(addressCache);
        rocksdb::WriteBatch batch;
        for (const auto &pair : cache) {
            rocksdb::Slice keySlice(reinterpret_cast<const char *>(&pair.first.key), sizeof(pair.first.key));
            rocksdb::Slice valueSlice(reinterpret_cast<const char *>(&pair.second), sizeof(pair.second));
            batch.Put(db.getColumn(type).get(), keySlice, valueSlice);
        }
        db.writeBatch(batch);
        cache.clear();
    }
    
public:
    
    HashIndexCreator(const ParserConfigurationBase &config, const std::string &path);
    ~HashIndexCreator();
    
    void processTx(const blocksci::RawTransaction *tx, uint32_t txNum, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts);
    
    template<blocksci::DedupAddressType::Enum type>
    void processScript(uint32_t equivNum, const blocksci::ScriptAccess &);
    
    
    void addTx(const blocksci::uint256 &hash, uint32_t txID);
    
    uint32_t getTxIndex(const blocksci::uint256 &txHash);
    
    template<blocksci::AddressType::Enum type>
    void addAddress(const typename blocksci::AddressInfo<type>::IDType &hash, uint32_t scriptNum) {
        auto &cache = std::get<HashIndexAddressCache<type>>(addressCache);
        cache.insert(hash, scriptNum);
        if (cache.isFull()) {
            clearAddressCache<type>();
        }
    }
    
    template<blocksci::AddressType::Enum type>
    uint32_t lookupAddress(const typename blocksci::AddressInfo<type>::IDType &hash) {
        auto &cache = std::get<HashIndexAddressCache<type>>(addressCache);
        auto it = cache.find(hash);
        if (it != cache.end()) {
            return it->second;
        } else {
            return db.lookupAddress<type>(hash);
        }
    }
    
    std::unique_ptr<rocksdb::Iterator> getIterator(blocksci::AddressType::Enum type) {
        return db.getIterator(type);
    }
    std::unique_ptr<rocksdb::Iterator> getIterator(blocksci::DedupAddressType::Enum type) {
        return db.getIterator(type);
    }
    std::unique_ptr<rocksdb::Iterator> getTxIterator() {
        return db.getTxIterator();
    }
    std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(blocksci::AddressType::Enum type) {
        return db.getColumn(type);
    }
    std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(blocksci::DedupAddressType::Enum type) {
        return db.getColumn(type);
    }
    std::unique_ptr<rocksdb::ColumnFamilyHandle> &getTxColumn() {
        return db.getTxColumn();
    }
    
    void writeBatch(rocksdb::WriteBatch &batch) {
        db.writeBatch(batch);
    }
    
    void rollback(const blocksci::State &state);
    
    void compact() {
        db.compactDB();
    }
};

#endif /* hash_index_creator_hpp */
