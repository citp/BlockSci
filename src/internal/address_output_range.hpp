//
//  address_output_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//

#ifndef address_output_range_h
#define address_output_range_h

#include <blocksci/core/core_fwd.hpp>

#include <range/v3/view/facade.hpp>

namespace rocksdb {
    class Iterator;
}

namespace blocksci {
    class AddressIndex;
    
    class RawAddressOutputRange : public ranges::view_facade<RawAddressOutputRange> {
        friend ranges::range_access;
        AddressIndex *index;
        
        struct cursor {
        private:
            AddressIndex *index;
            int rowNum = 0;
            int currentTypeIndex = -1;
            std::unique_ptr<rocksdb::Iterator> it;
        public:
            cursor();
            explicit cursor(AddressIndex &index_);
            
            ~cursor();
            
            cursor(const cursor &other);
            
            cursor &operator=(const cursor &other);
            
            std::pair<RawAddress, InoutPointer> read() const;
            
            bool equal(ranges::default_sentinel_t) const;
            
            bool equal(cursor const &that) const {
                return rowNum == that.rowNum;
            }
            
            void next();
            
            void advanceToNext();
            void prev();
        };
        
        cursor begin_cursor() const {
            return cursor{*index};
        }
        
        ranges::default_sentinel_t end_cursor() const {
            return {};
        }
        
    public:
        RawAddressOutputRange() = default;
        RawAddressOutputRange(AddressIndex &index_) : index(&index_) {}
    };
}

#endif /* address_output_range_h */
