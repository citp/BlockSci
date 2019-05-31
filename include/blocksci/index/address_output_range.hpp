//
//  address_output_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//

#ifndef address_output_range_h
#define address_output_range_h

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>

#include <range/v3/view_facade.hpp>

namespace rocksdb {
    class Iterator;
}

namespace blocksci {
    class DataAccess;
    
    class BLOCKSCI_EXPORT AddressOutputRange : public ranges::view_facade<AddressOutputRange> {
        friend ranges::range_access;
        DataAccess *access;
        
        struct cursor {
        private:
            DataAccess *access;
            int rowNum = 0;
            int currentTypeIndex = -1;
            std::unique_ptr<rocksdb::Iterator> it;
        public:
            cursor();
            explicit cursor(DataAccess &access_);
            
            ~cursor();
            
            cursor(const cursor &other);
            
            cursor &operator=(const cursor &other);
            
            std::pair<Address, OutputPointer> read() const;
            
            bool equal(ranges::default_sentinel) const;
            
            bool equal(cursor const &that) const {
                return rowNum == that.rowNum;
            }
            
            void next();
            
            void advanceToNext();
            void prev();
        };
        
        cursor begin_cursor() const {
            return cursor{*access};
        }
        
        ranges::default_sentinel end_cursor() const {
            return {};
        }
        
    public:
        AddressOutputRange() = default;
        AddressOutputRange(DataAccess &access_) : access(&access_) {}
    };
}

#endif /* address_output_range_h */
