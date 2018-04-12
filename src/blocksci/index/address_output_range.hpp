//
//  address_output_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/26/18.
//

#ifndef address_output_range_hpp
#define address_output_range_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/util/data_access.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/index/address_index.hpp>

#include <range/v3/view_facade.hpp>

#include <rocksdb/iterator.h>
#include <iostream>

namespace blocksci {
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
            cursor() : access(nullptr), it(nullptr) {}
            explicit cursor(DataAccess &access_) : access(&access_), it(nullptr) {
                advanceToNext();
            }
            
            cursor(const cursor &other) : access(other.access), rowNum(other.rowNum), currentTypeIndex(other.currentTypeIndex) {
                if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
                    it = access->addressIndex.getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                    it->SeekToFirst();
                } else {
                    it.reset(nullptr);
                }
            }
            
            cursor &operator=(const cursor &other) {
                access = other.access;
                rowNum = other.rowNum;
                currentTypeIndex = other.currentTypeIndex;
                if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
                    it = access->addressIndex.getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                    it->SeekToFirst();
                } else {
                    it.reset(nullptr);
                }
                return *this;
            }
            
            std::pair<Address, OutputPointer> read() const {
                assert(it->Valid());
                auto key = it->key();
                uint32_t addressNum;
                OutputPointer outPoint;
                memcpy(&addressNum, key.data(), sizeof(addressNum));
                key.remove_prefix(sizeof(addressNum));
                memcpy(&outPoint, key.data(), sizeof(outPoint));
                return std::make_pair(Address{addressNum, static_cast<AddressType::Enum>(currentTypeIndex), *access}, outPoint);
            }
            
            bool equal(ranges::default_sentinel) const { return !it->Valid() && currentTypeIndex == AddressType::size; }
            
            bool equal(cursor const &that) const {
                return rowNum == that.rowNum;
            }
            
            void next() {
                it->Next();
                rowNum++;
                if (!it->Valid()) {
                    advanceToNext();
                }
            }
            
            void advanceToNext() {
                while (it == nullptr || !it->Valid()) {
                    currentTypeIndex++;
                    if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
                        it = access->addressIndex.getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                        it->SeekToFirst();
                    } else {
                        it.reset(nullptr);
                        break;
                    }
                }
            }
            
            void prev() {
                it->Prev();
                rowNum--;
                if (!it->Valid()) {
                    if (currentTypeIndex > 0) {
                        currentTypeIndex--;
                        it = access->addressIndex.getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                        it->SeekToLast();
                    } else {
                        it.reset(nullptr);
                    }
                }
            }
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

#endif /* address_output_range_hpp */
