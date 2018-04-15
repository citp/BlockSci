//
//  column_iterator.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#ifndef column_iterator_h
#define column_iterator_h

#include <blocksci/blocksci_export.h>

#include <blocksci/util/memory_view.hpp>

#include <range/v3/view_facade.hpp>

namespace rocksdb {
    class DB;
    class ColumnFamilyHandle;
    class Iterator;
}

namespace blocksci {
    class BLOCKSCI_EXPORT ColumnIterator : public ranges::view_facade<ColumnIterator> {
        friend ranges::range_access;
        rocksdb::DB *db;
        rocksdb::ColumnFamilyHandle *column;
        
        struct cursor {
        private:
            rocksdb::DB *db;
            rocksdb::ColumnFamilyHandle *column;
            std::unique_ptr<rocksdb::Iterator> it;
        public:
            cursor();
            explicit cursor(rocksdb::DB *db_, rocksdb::ColumnFamilyHandle *column_);
            cursor(const cursor &other);
            cursor &operator=(const cursor &other);
            ~cursor();
            
            std::pair<MemoryView, MemoryView> read() const;
            
            bool equal(ranges::default_sentinel) const;
            
            void next();
            void prev();
        };
        
        cursor begin_cursor() const {
            return cursor{db, column};
        }
        
        ranges::default_sentinel end_cursor() const {
            return {};
        }
        
    public:
        ColumnIterator() = default;
        ColumnIterator(rocksdb::DB *db_, rocksdb::ColumnFamilyHandle *column_) : db(db_), column(column_) {}
    };
}



#endif /* column_iterator_h */
