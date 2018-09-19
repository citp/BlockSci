//
//  input_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blocksci_input_range_hpp
#define blocksci_input_range_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/input.hpp>

#include <range/v3/range_concepts.hpp>

namespace blocksci {
    class ChainAccess;
    class DataAccess;
    
    struct BLOCKSCI_EXPORT InputRange {
        const Inout *inouts;
        const uint16_t *spentOutputNums;
        const uint32_t *sequenceNumbers;
        BlockHeight height;
        uint32_t txIndex;
        uint16_t maxInputNum;
        uint32_t maxTxCount;
        DataAccess *access = nullptr;

        struct iterator {
            using self_type = iterator;
            using value_type = Input;
            using pointer = Input;
            using reference = Input;
            using difference_type = int;
            using iterator_category = std::random_access_iterator_tag;

            const Inout *inouts;
            const uint16_t *spentOutputNums;
            const uint32_t *sequenceNumbers;
            BlockHeight height;
            uint32_t txIndex;
            uint16_t inputNum;
            uint32_t maxTxCount;
            DataAccess *access = nullptr;

            self_type &operator+=(difference_type i) { inputNum += i; return *this; }
            self_type &operator-=(difference_type i) { inputNum -= i; return *this; }
            self_type &operator++() { ++inputNum; return *this; }
            self_type &operator--() { --inputNum; return *this; }
            self_type operator++(int) {  self_type tmp = *this; this->operator++(); return tmp; }
            self_type operator--(int) { self_type tmp = *this; this->operator--(); return tmp; }
            self_type operator+(difference_type i) const {  self_type tmp = *this; tmp += i; return tmp; }
            self_type operator-(difference_type i) const { self_type tmp = *this; tmp -= i; return tmp; }
            
            value_type operator*() const { return {{txIndex, inputNum}, height, inouts[inputNum], &spentOutputNums[inputNum], &sequenceNumbers[inputNum], maxTxCount, *access}; }
            value_type operator[](difference_type i) const {
                auto index = static_cast<uint16_t>(static_cast<int>(inputNum) + i);
                return {{txIndex, index}, height, inouts[index], &spentOutputNums[inputNum], &sequenceNumbers[index], maxTxCount, *access};
            }
            
            bool operator==(const self_type& rhs) const { return inputNum == rhs.inputNum; }
            bool operator!=(const self_type& rhs) const { return inputNum != rhs.inputNum; }
            bool operator<(const self_type& rhs) const { return inputNum < rhs.inputNum; }
            bool operator>(const self_type& rhs) const { return inputNum > rhs.inputNum; }
            bool operator<=(const self_type& rhs) const { return inputNum <= rhs.inputNum; }
            bool operator>=(const self_type& rhs) const { return inputNum >= rhs.inputNum; }
            
            difference_type operator-(const self_type& it) const { return static_cast<int>(inputNum - it.inputNum); }
        };

        iterator begin() const {
            return iterator{inouts, spentOutputNums, sequenceNumbers, height, txIndex, 0, maxTxCount, access};
        }

        iterator end() const {
            return iterator{nullptr, nullptr, nullptr, height, txIndex, maxInputNum, maxTxCount, nullptr};
        }

        uint16_t size() const {
            return maxInputNum;
        }

        Input operator[](uint16_t inputNum) {
            return {{txIndex, inputNum}, height, inouts[inputNum], &spentOutputNums[inputNum], &sequenceNumbers[inputNum], maxTxCount, *access};
        }
    };
    
    inline InputRange::iterator::self_type BLOCKSCI_EXPORT operator+(InputRange::iterator::difference_type i, const InputRange::iterator &it) {
        return it + i;
    }
    
    CONCEPT_ASSERT(ranges::BidirectionalRange<InputRange>());
    CONCEPT_ASSERT(ranges::BidirectionalIterator<InputRange::iterator>());
    CONCEPT_ASSERT(ranges::SizedSentinel<InputRange::iterator, InputRange::iterator>());
    CONCEPT_ASSERT(ranges::TotallyOrdered<InputRange::iterator>());
    CONCEPT_ASSERT(ranges::RandomAccessIterator<InputRange::iterator>());
    CONCEPT_ASSERT(ranges::RandomAccessRange<InputRange>());
    CONCEPT_ASSERT(ranges::SizedRange<InputRange>());
}

#endif /* blocksci_input_range_hpp */
