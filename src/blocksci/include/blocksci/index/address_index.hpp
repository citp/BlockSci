//
//  address_index.hpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#ifndef address_index_hpp
#define address_index_hpp

#include "column_iterator.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/address/address_info.hpp>

#include <range/v3/view_facade.hpp>
#include <range/v3/view/transform.hpp>

#include <cstring>
#include <unordered_set>
#include <string>
#include <vector>

namespace rocksdb {
    class Iterator;
}

namespace blocksci {
    class DataAccess;
    class EquivAddress;
    class AddressOutputRange;
    struct State;
    class AddressIndexPriv;
    
    class BLOCKSCI_EXPORT AddressIndex {
        friend class AddressOutputRange;
        
        std::unique_ptr<AddressIndexPriv> impl;
        
        std::unordered_set<Address> getPossibleNestedEquivalentUp(const Address &address) const;
        std::unordered_set<Address> getPossibleNestedEquivalentDown(const Address &address) const;
        
        ColumnIterator getRawOutputPointerRange(const Address &address) const;
    public:
        
        AddressIndex(const std::string &path, bool readonly);
        ~AddressIndex();

        bool checkIfExists(const Address &address) const;
        bool checkIfTopLevel(const Address &address) const;
        
        auto getOutputPointers(const Address &address) const {
            return getRawOutputPointerRange(address) | ranges::view::transform([](std::pair<MemoryView, MemoryView> pair) -> OutputPointer {
                auto &key = pair.first;
                key.data += sizeof(uint32_t);
                OutputPointer outPoint;
                memcpy(&outPoint, key.data, sizeof(outPoint));
                return outPoint;
            });
        }
        
        std::vector<Address> getPossibleNestedEquivalent(const Address &address) const;
        std::vector<Address> getIncludingMultisigs(const Address &searchAddress) const;
        
        void addNestedAddresses(std::vector<std::pair<blocksci::RawAddress, blocksci::DedupAddress>> nestedCache);
        void addOutputAddresses(std::vector<std::pair<blocksci::RawAddress, blocksci::OutputPointer>> outputCache);
        
        void compactDB();
        
        void rollback(const State &state);
    };
    
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


#endif /* address_index_hpp */
