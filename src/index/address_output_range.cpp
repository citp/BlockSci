//
//  address_output_range.cpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 4/18/23.
//
//

#include "address_index_priv.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/index/address_output_range.hpp>
#include <blocksci/util/data_access.hpp>

namespace blocksci {
    AddressOutputRange::cursor::cursor() : access(nullptr), it(nullptr) {}
    
    AddressOutputRange::cursor::cursor(DataAccess &access_) : access(&access_), it(nullptr) {
        advanceToNext();
    }
    
    AddressOutputRange::cursor::cursor(const cursor &other) : access(other.access), rowNum(other.rowNum), currentTypeIndex(other.currentTypeIndex) {
        if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
            it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
            it->SeekToFirst();
        } else {
            it.reset(nullptr);
        }
    }
    
    AddressOutputRange::cursor::~cursor() = default;
    
    AddressOutputRange::cursor &AddressOutputRange::cursor::operator=(const cursor &other) {
        access = other.access;
        rowNum = other.rowNum;
        currentTypeIndex = other.currentTypeIndex;
        if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
            it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
            it->SeekToFirst();
        } else {
            it.reset(nullptr);
        }
        return *this;
    }
    
    std::pair<Address, OutputPointer> AddressOutputRange::cursor::read() const {
        auto key = it->key();
        uint32_t addressNum;
        OutputPointer outPoint;
        memcpy(&addressNum, key.data(), sizeof(addressNum));
        key.remove_prefix(sizeof(addressNum));
        memcpy(&outPoint, key.data(), sizeof(outPoint));
        return std::make_pair(Address{addressNum, static_cast<AddressType::Enum>(currentTypeIndex), *access}, outPoint);
    }
    
    void AddressOutputRange::cursor::advanceToNext() {
        while (it == nullptr || !it->Valid()) {
            currentTypeIndex++;
            if (static_cast<size_t>(currentTypeIndex) < AddressType::size) {
                it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                it->SeekToFirst();
            } else {
                it.reset(nullptr);
                break;
            }
        }
    }
    
    bool AddressOutputRange::cursor::equal(ranges::default_sentinel) const {
        return !it->Valid() && currentTypeIndex == AddressType::size;
    }
    
    void AddressOutputRange::cursor::next() {
        it->Next();
        rowNum++;
        if (!it->Valid()) {
            advanceToNext();
        }
    }
    
    void AddressOutputRange::cursor::prev() {
        it->Prev();
        rowNum--;
        if (!it->Valid()) {
            if (currentTypeIndex > 0) {
                currentTypeIndex--;
                it = access->addressIndex.impl->getOutputIterator(static_cast<AddressType::Enum>(currentTypeIndex));
                it->SeekToLast();
            } else {
                it.reset(nullptr);
            }
        }
    }
}

