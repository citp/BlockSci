//
//  address_output_range.cpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 4/18/23.
//
//

#include "address_output_range.hpp"
#include "address_index.hpp"

#include <blocksci/core/inout_pointer.hpp>
#include <blocksci/core/raw_address.hpp>

#include <rocksdb/db.h>

namespace blocksci {
    RawAddressOutputRange::cursor::cursor() : index(nullptr), it(nullptr) {}
    
    RawAddressOutputRange::cursor::cursor(AddressIndex &index_) : index(&index_), it(nullptr) {
        advanceToNext();
    }
    
    RawAddressOutputRange::cursor::cursor(const cursor &other) : index(other.index), rowNum(other.rowNum), currentTypeIndex(other.currentTypeIndex) {
        if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
            it = index->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
            it->SeekToFirst();
        } else {
            it.reset(nullptr);
        }
    }
    
    RawAddressOutputRange::cursor::~cursor() = default;
    
    RawAddressOutputRange::cursor &RawAddressOutputRange::cursor::operator=(const cursor &other) {
        index = other.index;
        rowNum = other.rowNum;
        currentTypeIndex = other.currentTypeIndex;
        if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
            it = index->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
            it->SeekToFirst();
        } else {
            it.reset(nullptr);
        }
        return *this;
    }
    
    std::pair<RawAddress, InoutPointer> RawAddressOutputRange::cursor::read() const {
        auto key = it->key();
        uint32_t addressNum;
        InoutPointer outPoint;
        memcpy(&addressNum, key.data(), sizeof(addressNum));
        key.remove_prefix(sizeof(addressNum));
        memcpy(&outPoint, key.data(), sizeof(outPoint));
        return std::make_pair(RawAddress{addressNum, static_cast<AddressType::Enum>(currentTypeIndex)}, outPoint);
    }
    
    void RawAddressOutputRange::cursor::advanceToNext() {
        while (it == nullptr || !it->Valid()) {
            currentTypeIndex++;
            if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
                it = index->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                it->SeekToFirst();
            } else {
                it.reset(nullptr);
                break;
            }
        }
    }
    
    bool RawAddressOutputRange::cursor::equal(ranges::default_sentinel_t) const {
        return !it->Valid() && currentTypeIndex == AddressType::size;
    }
    
    void RawAddressOutputRange::cursor::next() {
        it->Next();
        rowNum++;
        if (!it->Valid()) {
            advanceToNext();
        }
    }
    
    void RawAddressOutputRange::cursor::prev() {
        it->Prev();
        rowNum--;
        if (!it->Valid()) {
            if (currentTypeIndex > 0) {
                currentTypeIndex--;
                it = index->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                it->SeekToLast();
            } else {
                it.reset(nullptr);
            }
        }
    }
}

