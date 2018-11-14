//
//  blocksci_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_blocksci_range_h
#define blocksci_blocksci_range_h

#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

template <typename T>
struct RangeClasses {
	pybind11::class_<Sequence<T>> sequence;
    pybind11::class_<Iterator<T>, Sequence<T>, GenericIterator> iterator;
    pybind11::class_<Range<T>, Sequence<T>, GenericRange> range;
};

#endif /* blocksci_blocksci_range_h */
