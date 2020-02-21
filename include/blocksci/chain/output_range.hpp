//
//  output_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blocksci_output_range_hpp
#define blocksci_output_range_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/output.hpp>

#include <range/v3/range/concepts.hpp>

namespace blocksci {
    class ChainAccess;
    class DataAccess;
    
    struct BLOCKSCI_EXPORT OutputRange {
        /** Pointer to the Inout of the first input in the range, more <Inout>s follow and can be accessed like inouts[number] */
        const Inout *inouts;

        /** Block height of the transaction whose outputs are part of the range */
        BlockHeight height;

        /** Transaction number of the outputs that are part of the range */
        uint32_t txIndex;

        /** Maximum (last) tx-internal input number that is part of the range */
        uint16_t maxOutputNum;

        /** Needed to create Output objects in the iterator struct */
        uint32_t maxTxLoaded;

        DataAccess *access = nullptr;

        struct iterator {
            using self_type = iterator;
            using value_type = Output;
            using pointer = Output;
            using reference = Output;
            using difference_type = int;
            using iterator_category = std::random_access_iterator_tag;

            const Inout *inouts;
            BlockHeight height;
            uint32_t txIndex;
            uint16_t outputNum;
            uint32_t maxTxLoaded;
            DataAccess *access = nullptr;
            
            iterator() = default;
            iterator(const Inout *inouts_, BlockHeight height_, uint32_t txIndex_, uint16_t outputNum_, uint32_t maxTxLoaded_, DataAccess *access_) : inouts(inouts_), height(height_), txIndex(txIndex_), outputNum(outputNum_), maxTxLoaded(maxTxLoaded_), access(access_) {}

            self_type &operator+=(difference_type i) { outputNum += i; return *this; }
            self_type &operator-=(difference_type i) { outputNum -= i; return *this; }
            self_type &operator++() { ++outputNum; return *this; }
            self_type &operator--() { --outputNum; return *this; }
            self_type operator++(int) {  self_type tmp = *this; this->operator++(); return tmp; }
            self_type operator--(int) { self_type tmp = *this; this->operator--(); return tmp; }
            self_type operator+(difference_type i) const {  self_type tmp = *this; tmp += i; return tmp; }
            self_type operator-(difference_type i) const { self_type tmp = *this; tmp -= i; return tmp; }
            
            value_type operator*() const { return {{txIndex, outputNum}, height, inouts[outputNum], maxTxLoaded, *access}; }
            value_type operator[](difference_type i) const { return {{txIndex, static_cast<uint16_t>(static_cast<int>(outputNum) + i)}, height, inouts[outputNum], maxTxLoaded, *access}; }
            
            bool operator==(const self_type& rhs) const { return outputNum == rhs.outputNum; }
            bool operator!=(const self_type& rhs) const { return outputNum != rhs.outputNum; }
            bool operator<(const self_type& rhs) const { return outputNum < rhs.outputNum; }
            bool operator>(const self_type& rhs) const { return outputNum > rhs.outputNum; }
            bool operator<=(const self_type& rhs) const { return outputNum <= rhs.outputNum; }
            bool operator>=(const self_type& rhs) const { return outputNum >= rhs.outputNum; }
            
            difference_type operator-(const self_type& it) const { return static_cast<int>(outputNum) - static_cast<int>(it.outputNum); }
        };

        iterator begin() const {
            return iterator{inouts, height, txIndex, 0, maxTxLoaded, access};
        }

        iterator end() const {
            return iterator{nullptr, height, txIndex, maxOutputNum, maxTxLoaded, nullptr};
        }

        uint16_t size() const {
            return maxOutputNum;
        }

        Output operator[](uint16_t outputNum) {
            return {{txIndex, outputNum}, height, inouts[outputNum], maxTxLoaded, *access};
        }
    };
    
    inline OutputRange::iterator::self_type BLOCKSCI_EXPORT operator+(OutputRange::iterator::difference_type i, const OutputRange::iterator &it) {
        return it + i;
    }
    
    CPP_assert(ranges::bidirectional_range<OutputRange>);
    CPP_assert(ranges::bidirectional_iterator<OutputRange::iterator>);
    CPP_assert(ranges::sized_sentinel_for<OutputRange::iterator, OutputRange::iterator>);
    CPP_assert(ranges::totally_ordered<OutputRange::iterator>);
    CPP_assert(ranges::random_access_iterator<OutputRange::iterator>);
    CPP_assert(ranges::random_access_range<OutputRange>);
    CPP_assert(ranges::sized_range<OutputRange>);
}

#endif /* blocksci_output_range_hpp */
