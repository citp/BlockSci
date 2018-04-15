//
//  address_index_priv.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#include "address_index_priv.hpp"

#include <blocksci/util/for_each.hpp>
#include <blocksci/address/address_info.hpp>

#include <sstream>

namespace blocksci {
    
    AddressIndexPriv::AddressIndexPriv(const std::string &path, bool readonly) {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        blocksci::for_each(AddressType::all(), [&](auto tag) {
            std::stringstream ss;
            ss << addressName(tag) << "_output";
            columnDescriptors.emplace_back(ss.str(), rocksdb::ColumnFamilyOptions{});
        });
        blocksci::for_each(AddressType::all(), [&](auto tag) {
            std::stringstream ss;
            ss << addressName(tag) << "_nested";
            columnDescriptors.emplace_back(ss.str(), rocksdb::ColumnFamilyOptions{});
        });
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());
        
        rocksdb::DB *dbPtr;
        std::vector<rocksdb::ColumnFamilyHandle *> columnHandlePtrs;
        if (readonly) {
            rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            assert(s.ok());
        } else {
            rocksdb::Status s = rocksdb::DB::Open(options, path.c_str(), columnDescriptors, &columnHandlePtrs, &dbPtr);
            assert(s.ok());
        }
        db = std::unique_ptr<rocksdb::DB>(dbPtr);
        for (auto handle : columnHandlePtrs) {
            columnHandles.emplace_back(std::unique_ptr<rocksdb::ColumnFamilyHandle>(handle));
        }
    }
    
    void AddressIndexPriv::compactDB() {
        for (auto &column : columnHandles) {
            db->CompactRange(rocksdb::CompactRangeOptions{}, column.get(), nullptr, nullptr);
        }
    }
    
    const std::unique_ptr<rocksdb::ColumnFamilyHandle> &AddressIndexPriv::getOutputColumn(AddressType::Enum type) const {
        return columnHandles[static_cast<size_t>(type)];
    }
    
    const std::unique_ptr<rocksdb::ColumnFamilyHandle> &AddressIndexPriv::getNestedColumn(AddressType::Enum type) const {
        return columnHandles[AddressType::size + static_cast<size_t>(type)];
    }
}
