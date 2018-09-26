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
    pybind11::class_<ranges::any_view<T>> iterator;
    pybind11::class_<ranges::any_view<T, ranges::category::random_access | ranges::category::sized>> range;
    pybind11::class_<ranges::any_view<ranges::optional<T>>> optionalIterator;
    pybind11::class_<ranges::any_view<ranges::optional<T>, ranges::category::random_access | ranges::category::sized>> optionalRange;

    RangeClasses(pybind11::module &m) : 
    iterator(m, strdup(PythonTypeName<ranges::any_view<T>>::name().c_str()), pybind11::dynamic_attr()),
    range(m, strdup(PythonTypeName<ranges::any_view<T, ranges::category::random_access | ranges::category::sized>>::name().c_str()), pybind11::dynamic_attr()),
    optionalIterator(m, strdup(PythonTypeName<ranges::any_view<ranges::optional<T>>>::name().c_str()), pybind11::dynamic_attr()),
    optionalRange(m, strdup(PythonTypeName<ranges::any_view<ranges::optional<T>, ranges::category::random_access | ranges::category::sized>>::name().c_str()), pybind11::dynamic_attr()) {}
};

#endif /* blocksci_blocksci_range_h */
