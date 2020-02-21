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

#include <range/v3/range/concepts.hpp>

namespace blocksci {
    class ChainAccess;
    class DataAccess;
    
    struct BLOCKSCI_EXPORT InputRange {
        /** Pointer to the Inout of the first input in the range, more <Inout>s follow and can be accessed like inouts[number] */
        const Inout *inouts;

        /** Pointer to the tx-internal output number that the first input in the range spends */
        const uint16_t *spentOutputNums;

        /** Pointer to the blockchain field sequence number of the first input in the range */
        const uint32_t *sequenceNumbers;

        /** Block height of the transaction whose inputs are part of the range */
        BlockHeight height;

        /** Transaction number of the inputs that are part of the range */
        uint32_t txIndex;

        /** Maximum tx-internal input number that is part of the range */
        uint16_t maxInputNum;

        /** Needed to create Input objects in the iterator struct */
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

            /** Parameters of Input constructor: InputPointer &pointer_, BlockHeight blockHeight_, Inout &inout_, uint16_t *spentOutputNum_, uint32_t *sequenceNum_, uint32_t maxTxCount_, DataAccess &access_ */
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
    
    CPP_assert(ranges::bidirectional_range<InputRange>);
    CPP_assert(ranges::bidirectional_iterator<InputRange::iterator>);
    CPP_assert(ranges::sized_sentinel_for<InputRange::iterator, InputRange::iterator>);
    CPP_assert(ranges::totally_ordered<InputRange::iterator>);
    CPP_assert(ranges::random_access_iterator<InputRange::iterator>);
    CPP_assert(ranges::random_access_range<InputRange>);
    CPP_assert(ranges::sized_range<InputRange>);
}

#endif /* blocksci_input_range_hpp */
