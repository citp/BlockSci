//
//  blocksci_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_blocksci_range_create_h
#define blocksci_blocksci_range_create_h

#include "blocksci_range.hpp"
#include "method_types.hpp"

template <typename T>
RangeClasses<T> createRangeClasses(pybind11::module &m) {
    pybind11::class_<Sequence<T>> sequence(m, strdup(typenameLookup().getName<Sequence<T>>().c_str()), pybind11::dynamic_attr());
    pybind11::class_<Iterator<T>, Sequence<T>, GenericIterator> iterator(m, strdup(typenameLookup().getName<Iterator<T>>().c_str()), pybind11::dynamic_attr());
    pybind11::class_<Range<T>, Sequence<T>, GenericRange> range(m, strdup(typenameLookup().getName<Range<T>>().c_str()), pybind11::dynamic_attr());
    return {sequence, iterator, range};
}

template <typename T>
RangeClasses<T> createAddressRangeClasses(pybind11::module &m) {
    pybind11::class_<Sequence<T>> sequence(m, strdup(typenameLookup().getName<Sequence<T>>().c_str()), pybind11::dynamic_attr());
    pybind11::class_<Iterator<T>, Sequence<T>, GenericAddressIterator> iterator(m, strdup(typenameLookup().getName<Iterator<T>>().c_str()), pybind11::dynamic_attr());
    pybind11::class_<Range<T>, Sequence<T>, GenericAddressRange> range(m, strdup(typenameLookup().getName<Range<T>>().c_str()), pybind11::dynamic_attr());
    return {sequence, iterator, range};
}

#endif /* blocksci_blocksci_range_create_h */
