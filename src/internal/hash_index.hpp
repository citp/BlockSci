//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef blocksci_index_hash_index_hpp
#define blocksci_index_hash_index_hpp

#include "address_info.hpp"
#include "memory_view.hpp"

#include <range/v3/view/any_view.hpp>
#include <range/v3/utility/optional.hpp>

#include <rocksdb/db.h>

#include <wjfilesystem/path.h>

#include <vector>
#include <cstdint>
#include <cstring>

namespace blocksci {

    /** Provides access to hash indexes (RocksDB database)
     *
     * This RocksDB database is a lookup table from both (tx hash) and (address hash) to (internal BlockSci index) for those objects.
     *
     * The hash index contains a lookup table for each address type as well as one for transactions.
     * In each of these tables, the key is the standard public identifier (tx hash for transactions, pubkey hash
     * for pay to pubkey hash addresses, etc.) and the value is the internal integer index of that object.
     *
     * ColumnDescriptors: One for every named AddressType (@see blocksci::AddressType::all() @see blocksci::addressName())
     *                    and "T" for the txHash -> txNum database
     *                    Eg. "T", "pubkey", "pubkeyhash", "multisig_pubkey" etc.
     *
     * Key/value format:
     *     - ColumnFamily "T": transaction hash -> transaction number
     *         + Key: blocksci::uint256 hash
     *         + Value: uint32_t txNum
     *     - Other column families: address identifier (eg. pubkeyhash or scripthash) -> script number
     *         + Key: blocksci::uint160 or blocksci::uint256
     *         + Value: uint32_t scriptNum
     *
     * Directory: hashIndex/
     */
    class HashIndex {
        /** Pointer to the RocksDB instance */
        std::unique_ptr<rocksdb::DB> db;

        /** RocksDB column handles, one for each address type, @see blocksci::AddressType::Enum */
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;
        
        ranges::optional<uint32_t> lookupAddressImpl(AddressType::Enum type, const char *data, size_t size);
        void addAddressesImpl(AddressType::Enum type, std::vector<std::pair<MemoryView, MemoryView>> dataViews);
        
        template <typename T>
        ranges::optional<uint32_t> getMatch(rocksdb::ColumnFamilyHandle *handle, const T &t) {
            rocksdb::PinnableSlice val;
            rocksdb::Slice key{reinterpret_cast<const char *>(&t), sizeof(t)};
            auto getStatus = db->Get(rocksdb::ReadOptions{}, handle, key, &val);
            if (getStatus.ok()) {
                uint32_t value;
                memcpy(&value, val.data(), sizeof(value));
                return value;
            } else {
                return ranges::nullopt;
            }
        }

        /** Get the scriptNum for the given AddressType and identifier (pubkeyhash, script hash etc.) */
        ranges::optional<uint32_t> getAddressMatch(blocksci::AddressType::Enum type, const char *data, size_t size) {
            rocksdb::PinnableSlice val;
            rocksdb::Slice key{data, size};
            auto getStatus = db->Get(rocksdb::ReadOptions{}, getColumn(type).get(), key, &val);
            if (getStatus.ok()) {
                uint32_t value;
                memcpy(&value, val.data(), sizeof(value));
                return value;
            } else {
                return ranges::nullopt;
            }
        }
        
        void addAddresses(AddressType::Enum type, std::vector<std::pair<MemoryView, MemoryView>> dataViews) {
            rocksdb::WriteBatch batch;
            for (auto &pair : dataViews) {
                auto key = rocksdb::Slice(pair.first.data, pair.first.size);
                auto value = rocksdb::Slice(pair.second.data, pair.second.size);
                batch.Put(getColumn(type).get(), key, value);
            }
            writeBatch(batch);
        }
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(AddressType::Enum type) {
            auto index = static_cast<size_t>(type);
            return columnHandles.at(index);
        }
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getColumn(DedupAddressType::Enum type) {
            switch (type) {
                case DedupAddressType::PUBKEY:
                    return getColumn(AddressType::PUBKEYHASH);
                case DedupAddressType::SCRIPTHASH:
                    return getColumn(AddressType::SCRIPTHASH);
                case DedupAddressType::MULTISIG:
                    return getColumn(AddressType::MULTISIG);
                case DedupAddressType::NULL_DATA:
                    return getColumn(AddressType::NULL_DATA);
                case DedupAddressType::NONSTANDARD:
                    return getColumn(AddressType::NONSTANDARD);
                case DedupAddressType::WITNESS_UNKNOWN:
                    return getColumn(AddressType::WITNESS_UNKNOWN);
            }
            assert(false);
            return getColumn(AddressType::NONSTANDARD);
        }
        
        std::unique_ptr<rocksdb::ColumnFamilyHandle> &getTxColumn() {
            return columnHandles.back();
        }
        
        std::unique_ptr<rocksdb::Iterator> getIterator(AddressType::Enum type) {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getColumn(type).get())};
        }
        std::unique_ptr<rocksdb::Iterator> getIterator(DedupAddressType::Enum type) {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getColumn(type).get())};
        }
        std::unique_ptr<rocksdb::Iterator> getTxIterator() {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getTxColumn().get())};
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            rocksdb::WriteOptions options;
            options.disableWAL = true;
            db->Write(options, &batch);
        }
        
    public:
        
        HashIndex(const filesystem::path &path, bool readonly);
        ~HashIndex();

        template<AddressType::Enum type>
        ranges::optional<uint32_t> lookupAddress(const typename AddressInfo<type>::IDType &hash) {
            return lookupAddressImpl(type, reinterpret_cast<const char *>(&hash), sizeof(hash));
        }

        /** Get the scriptNum for the given public key hash */
        ranges::optional<uint32_t> getPubkeyHashIndex(const uint160 &pubkeyhash);
      
        /** Get the scriptNum for the given script hash */
        ranges::optional<uint32_t> getScriptHashIndex(const uint160 &scripthash);
        ranges::optional<uint32_t> getScriptHashIndex(const uint256 &scripthash);
      
        /** Get the tx number for the given transaction hash */
        ranges::optional<uint32_t> getTxIndex(const uint256 &txHash);
        
        uint32_t countColumn(AddressType::Enum type);
        uint32_t countTxes();
        
        template<AddressType::Enum type>
        void addAddresses(std::vector<std::pair<typename blocksci::AddressInfo<type>::IDType, uint32_t>> rows) {
            std::vector<std::pair<MemoryView, MemoryView>> dataViews;
            for (const auto &pair : rows) {
                dataViews.emplace_back(
                MemoryView{reinterpret_cast<const char *>(&pair.first), sizeof(pair.first)},
                MemoryView{reinterpret_cast<const char *>(&pair.second), sizeof(pair.second)});
            }
            addAddressesImpl(type, dataViews);
        }

        /** Add a mapping from tx hash to tx number to the hash index for all given rows */
        void addTxes(std::vector<std::pair<uint256, uint32_t>> rows);
        
        void rollback(uint32_t txCount, const std::array<uint32_t, DedupAddressType::size> &scriptCounts);
        
        ranges::any_view<std::pair<MemoryView, MemoryView>> getRawAddressRange(AddressType::Enum type);
        
        template<AddressType::Enum type>
        ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<type>::IDType>> getAddressRange();

        /** Compact the underlying RocksDB database */
        void compactDB();
    };
    
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::PUBKEY>::IDType>> HashIndex::getAddressRange<AddressType::PUBKEY>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::PUBKEYHASH>::IDType>> HashIndex::getAddressRange<AddressType::PUBKEYHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::WITNESS_PUBKEYHASH>::IDType>> HashIndex::getAddressRange<AddressType::WITNESS_PUBKEYHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::MULTISIG_PUBKEY>::IDType>> HashIndex::getAddressRange<AddressType::MULTISIG_PUBKEY>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::SCRIPTHASH>::IDType>> HashIndex::getAddressRange<AddressType::SCRIPTHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::WITNESS_SCRIPTHASH>::IDType>> HashIndex::getAddressRange<AddressType::WITNESS_SCRIPTHASH>();
    extern template ranges::any_view<std::pair<uint32_t, typename blocksci::AddressInfo<AddressType::MULTISIG>::IDType>> HashIndex::getAddressRange<AddressType::MULTISIG>();
}

#endif /* blocksci_index_hash_index_hpp */
