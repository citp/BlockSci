//
//  generic_sequence.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/25/18.
//

#include "generic_sequence.hpp"
#include "blocksci_iterator_type.hpp"
#include "blocksci_range_type.hpp"

RawIterator<std::any> GenericIterator::getAllIterator() {
    return mpark::visit([](auto && seq) -> RawIterator<std::any> {
        return ranges::views::transform(std::forward<decltype(seq)>(seq), [](auto && item) -> std::any {
            return std::forward<decltype(item)>(item);
        });
    }, getGenericIterator().var);
}


RawRange<std::any> GenericRange::getAllRange() {
    return mpark::visit([](auto && seq) -> RawRange<std::any> {
        return ranges::views::transform(std::forward<decltype(seq)>(seq), [](auto && item) -> std::any {
            return std::forward<decltype(item)>(item);
        });
    }, getGenericRange().var);
}

BlocksciIteratorType GenericRange::getGenericIterator() {
    return getGenericRange().toIterator();
}
