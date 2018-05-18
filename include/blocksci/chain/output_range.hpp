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

namespace blocksci {
    class ChainAccess;
    class DataAccess;
    
    struct BLOCKSCI_EXPORT OutputRange {
        const Inout *inouts;
        BlockHeight height;
        uint32_t txIndex;
        uint16_t maxOutputNum;
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

            self_type &operator+=(difference_type i) { outputNum += i; return *this; }
            self_type &operator-=(difference_type i) { outputNum -= i; return *this; }
            self_type &operator++() { ++outputNum; return *this; }
            self_type &operator--() { --outputNum; return *this; }
            self_type operator++(int) {  self_type tmp = *this; this->operator++(); return tmp; }
            self_type operator--(int) { self_type tmp = *this; this->operator--(); return tmp; }
            self_type operator+(difference_type i) const {  self_type tmp = *this; tmp += i; return tmp; }
            self_type operator-(difference_type i) const { self_type tmp = *this; tmp -= i; return tmp; }
            value_type operator*() const { return {{txIndex, outputNum}, height, inouts[outputNum], maxTxLoaded, *access}; }
            bool operator==(const self_type& rhs) const {  return outputNum == rhs.outputNum; }
            bool operator!=(const self_type& rhs) const { return outputNum != rhs.outputNum; }
            difference_type operator-(const self_type& it) const { return static_cast<int>(outputNum - it.outputNum); }
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
}

#endif /* blocksci_output_range_hpp */
