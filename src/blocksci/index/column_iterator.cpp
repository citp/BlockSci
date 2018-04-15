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
    
    ColumnIterator::cursor::cursor(rocksdb::DB *db_, rocksdb::ColumnFamilyHandle *column_) : db(db_), column(column_), it(std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), column)}){
        it->SeekToFirst();
    }
    
    ColumnIterator::cursor::cursor(const cursor &other) : db(other.db), column(other.column), it(std::unique_ptr<rocksdb::Iterator>{db->NewIterator(rocksdb::ReadOptions(), column)}) {
        it->Seek(other.it->key());
    }
    
    ColumnIterator::cursor &ColumnIterator::cursor::operator=(const cursor &other) {
        it->Seek(other.it->key());
        return *this;
    }
    
    std::pair<MemoryView, MemoryView> ColumnIterator::cursor::read() const {
        auto key = it->key();
        auto value = it->value();
        return {{key.data(), key.size()}, {value.data(), value.size()}};
    }
    
    bool ColumnIterator::cursor::equal(ranges::default_sentinel) const { return !it->Valid(); }
    
    void ColumnIterator::cursor::next() { it->Next(); }
    void ColumnIterator::cursor::prev() { it->Prev(); }
}
