//
//  address_index.hpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#ifndef address_index_hpp
#define address_index_hpp

#include <blocksci/core/address_types.hpp>
#include <blocksci/core/core_fwd.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/utility/optional.hpp>

#include <rocksdb/db.h>

#include <wjfilesystem/path.h>

#include <memory>
#include <unordered_set>
#include <string>
#include <vector>

namespace blocksci {
    struct DedupAddress;
    class RawAddressOutputRange;

    /** Provides access to address indexes (RocksDB database)
     *
     * This RocksDB database stores information about which outputs a given address
     * is used in as well as information about how different addresses relate to each other.
     *
     * Directory: addressesDb/
     *
     * The address index contains two sets of tables:
     *
     * 1) The first set of tables contains a mapping of addresses to outputs that are sent to those addresses.
     *    The index is usually queried by scriptNum prefix to get all entries for one address.
     *    This index makes it each to access all locations on the Blockchain where a particular address occurs
     *    though this is significantly slower than operations that do not require access to the RocksDB database.
     *
     *    ColumnDescriptors: Every AddressType as its name and "_output" as a suffix. @see blocksci::AddressType::all() @see blocksci::addressName()
     *                       Eg. "pubkey_output", "pubkeyhash_output", "multisig_pubkey_output" etc.
     *
     *    Key/value format:
     *        - Key: uint32_t scriptNum, uint8_t[4] txNum, uint8_t[2] outputNumInTx
     *        - Value: <empty>
     *
     * 2) The second set of tables store information how certain address types are nested inside each other.
     *    The two places this occurs are with p2sh addresses wrapping other addresses and with multisig addresses containing pubkeys.
     *
     *    ColumnDescriptors: Every AddressType as its name and "_nested" as a suffix. @see blocksci::AddressType::all() @see blocksci::addressName()
     *                       Eg. "pubkey_nested", "pubkeyhash_nested", "multisig_pubkey_nested" etc.
     *
     *    Key/value format:
     *        - Keys: uint32_t child scriptNum, DedupAddress (= parent scriptNum, parent script type)
     *        - Value: <empty>
     */
    class AddressIndex {
        friend class RawAddressOutputRange;

        /** Pointer to the RocksDB instance */
        std::unique_ptr<rocksdb::DB> db;

        /** RocksDB column handles, one for every AddressType and the suffixes "_nested" and "_output", see above for details */
        std::vector<std::unique_ptr<rocksdb::ColumnFamilyHandle>> columnHandles;

        const std::unique_ptr<rocksdb::ColumnFamilyHandle> &getOutputColumn(AddressType::Enum type) const;
        const std::unique_ptr<rocksdb::ColumnFamilyHandle> &getNestedColumn(AddressType::Enum type) const;
        
        std::unique_ptr<rocksdb::Iterator> getOutputIterator(AddressType::Enum type) const {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getOutputColumn(type).get())};
        }
        
        std::unique_ptr<rocksdb::Iterator> getNestedIterator(AddressType::Enum type) const {
            return std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), getNestedColumn(type).get())};
        }
        
        void writeBatch(rocksdb::WriteBatch &batch) {
            rocksdb::WriteOptions options;
            options.disableWAL = true;
            db->Write(options, &batch);
        }
        
    public:
        
        AddressIndex(const filesystem::path &path, bool readonly);
        ~AddressIndex();

        /** Get InoutPointer objects for all outputs that belong to the given address */
        ranges::any_view<InoutPointer, ranges::category::forward> getOutputPointers(const RawAddress &address) const;
        
        std::vector<DedupAddress> getNestingScriptHash(const RawAddress &searchAddress) const;
        std::unordered_set<DedupAddress> getPossibleNestedEquivalentUp(const RawAddress &searchAddress) const;
        ranges::any_view<RawAddress> getIncludingMultisigs(const RawAddress &searchAddress) const;
        
        void addNestedAddresses(std::vector<std::pair<RawAddress, DedupAddress>> nestedCache);

        /** Add a link between the given address and the given output to the index
         *
         * Key format: scriptNum, txNum, outputNumInTx
         * Value format: <empty>
         */
        void addOutputAddresses(std::vector<std::pair<RawAddress, InoutPointer>> outputCache);

        /** Compact the underlying RocksDB database */
        void compactDB();
        
        void rollback(uint32_t txNum);
    };
}


#endif /* address_index_hpp */
