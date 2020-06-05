//
//  column_iterator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#ifndef column_iterator_h
#define column_iterator_h

#include "memory_view.hpp"

#include <range/v3/view/facade.hpp>

#include <rocksdb/db.h>

#include <vector>

namespace rocksdb {
    class DB;
    class ColumnFamilyHandle;
    class Iterator;
}

namespace blocksci {
    struct MemoryView;
    
    class ColumnIterator : public ranges::view_facade<ColumnIterator> {
        friend ranges::range_access;
        rocksdb::DB *db;
        rocksdb::ColumnFamilyHandle *column;
        std::vector<char> prefixBytes;
        
        struct cursor {
        private:
            rocksdb::DB *db;
            rocksdb::ColumnFamilyHandle *column;
            std::shared_ptr<rocksdb::Iterator> it;
            std::vector<char> prefixBytes;
        public:
            cursor() = default;
            cursor(rocksdb::DB *db_, rocksdb::ColumnFamilyHandle *column_, std::vector<char> prefix)  : db(db_), column(column_), it(std::shared_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), column)}), prefixBytes(std::move(prefix)) {
                if (prefixBytes.size() > 0) {
                    rocksdb::Slice key(prefixBytes.data(), prefixBytes.size());
                    it->Seek(key);
                } else {
                    it->SeekToFirst();
                }
            }
            
            std::pair<MemoryView, MemoryView> read() const {
                auto key = it->key();
                auto value = it->value();
                return {{key.data(), key.size()}, {value.data(), value.size()}};
            }
            
            bool equal(ranges::default_sentinel_t) const {
                rocksdb::Slice key(prefixBytes.data(), prefixBytes.size());
                return !it->Valid() || !it->key().starts_with(key);
            }
            
            bool equal(const cursor &other) const {
                return it->key().ToString() == other.it->key().ToString();
            }
            
            void next() {
                if (it.use_count() > 1) {
                    auto newIt = std::shared_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), column)};
                    newIt->Seek(it->key());
                    it = newIt;
                }
                it->Next();
            }
        };
        
        cursor begin_cursor() const {
            return cursor{db, column, prefixBytes};
        }
        
        ranges::default_sentinel_t end_cursor() const {
            return {};
        }
        
    public:
        ColumnIterator() = default;
        ColumnIterator(rocksdb::DB *db_, rocksdb::ColumnFamilyHandle *column_) : db(db_), column(column_) {}
        ColumnIterator(rocksdb::DB *db_, rocksdb::ColumnFamilyHandle *column_, std::vector<char> prefix) : db(db_), column(column_), prefixBytes(std::move(prefix)) {}
        
    };
}

#endif /* column_iterator_h */
