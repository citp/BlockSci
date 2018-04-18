//
//  column_iterator.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#include "column_iterator.hpp"

#include <rocksdb/db.h>

namespace blocksci {
    
    ColumnIterator::cursor::cursor() : it(nullptr) {}
    
    ColumnIterator::cursor::~cursor() = default;
    
    ColumnIterator::cursor::cursor(rocksdb::DB *db_, rocksdb::ColumnFamilyHandle *column_, std::vector<char> prefix) : db(db_), column(column_), it(std::shared_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), column)}), prefixBytes(std::move(prefix)) {
        if (prefixBytes.size() > 0) {
            rocksdb::Slice key(prefixBytes.data(), prefixBytes.size());
            it->Seek(key);
        } else {
            it->SeekToFirst();
        }
        
    }

    ColumnIterator::cursor::cursor(const cursor &other) : db(other.db), column(other.column), it(other.it), prefixBytes(other.prefixBytes) {}
    
    ColumnIterator::cursor::cursor(cursor &&other) : db(other.db), column(other.column), it(std::move(other.it)), prefixBytes(std::move(other.prefixBytes)) {}
    
    ColumnIterator::cursor &ColumnIterator::cursor::operator=(const cursor &other) {
        db = other.db;
        column = other.column;
        prefixBytes = other.prefixBytes;
        it = other.it;
        return *this;
    }
    
    ColumnIterator::cursor &ColumnIterator::cursor::operator=(cursor && other) {
        db = other.db;
        column = other.column;
        prefixBytes = std::move(other.prefixBytes);
        it = std::move(other.it);
        return *this;
    }
    
    std::pair<MemoryView, MemoryView> ColumnIterator::cursor::read() const {
        auto key = it->key();
        auto value = it->value();
        return {{key.data(), key.size()}, {value.data(), value.size()}};
    }
    
    bool ColumnIterator::cursor::equal(ranges::default_sentinel) const {
        rocksdb::Slice key(prefixBytes.data(), prefixBytes.size());
        return !it->Valid() || !it->key().starts_with(key);
    }
    
    void ColumnIterator::cursor::next() {
        if (it.use_count() > 1) {
            auto newIt = std::shared_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), column)};
            newIt->Seek(it->key());
            it = newIt;
        }
        it->Next();
    }
}
