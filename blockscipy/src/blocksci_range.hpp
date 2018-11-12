//
//  blocksci_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_blocksci_range_h
#define blocksci_blocksci_range_h

#include "method_types.hpp"

#include <pybind11/pybind11.h>

#include <range/v3/view/any_view.hpp>

template <typename T>
struct RangeClasses {
	pybind11::class_<Sequence<T>, GenericSequence> sequence;
    pybind11::class_<Iterator<T>, Sequence<T>> iterator;
    pybind11::class_<Range<T>, Sequence<T>> range;

    RangeClasses(pybind11::module &m) : 
    sequence(m, strdup(PythonTypeName<Sequence<T>>::name().c_str()), pybind11::dynamic_attr()),
    iterator(m, strdup(PythonTypeName<Iterator<T>>::name().c_str()), pybind11::dynamic_attr()),
    range(m, strdup(PythonTypeName<Range<T>>::name().c_str()), pybind11::dynamic_attr()) {}
};

#endif /* blocksci_blocksci_range_h */
