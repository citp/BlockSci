//
//  output_range_filters.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "output_range_py.hpp"
#include "caster_py.hpp"
#include "range_filter_apply_py.hpp"

#include <blocksci/chain/algorithms.hpp>

namespace py = pybind11;
using namespace blocksci;

template <typename Range>
struct AddOutputRangeMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(property_tag, "unspent", [](Range & range) {
            return outputsUnspent(range);
        }, "Returns a range including the subset of outputs which were never spent");
        func(method_tag, "spent_before_height",  [](Range &range, BlockHeight height) {
            return outputsSpentBeforeHeight(range, height);
        }, "Returns a range including the subset of outputs which were spent before the given height", py::arg("height"));
        func(method_tag, "spent_after_height",  [](Range &range, BlockHeight height) {
            return outputsSpentAfterHeight(range, height);
        }, "Returns a range including the subset of outputs which were spent after the given height", py::arg("height"));
        func(method_tag, "spent_with_age_less_than",  [](Range &range, BlockHeight height) {
            return outputsSpentWithinRelativeHeight(range, height);
        }, "Returns a range including the subset of outputs which were spent within the given number of blocks", py::arg("age"));
        func(method_tag, "spent_with_age_greater_than",  [](Range &range, BlockHeight height) {
            return outputsSpentOutsideRelativeHeight(range, height);
        }, "Returns a range including the subset of outputs which were spent later than the given number of blocks", py::arg("age"));
        func(method_tag, "with_type", [](Range &range, AddressType::Enum type) {
            return outputsOfType(range, type);
        }, "Returns a range including the subset of outputs which were sent to the given address type", py::arg("type"));
    }
};

void applyRangeFiltersToOutputRange(RangeClasses<Output> &classes) {
    applyRangeFiltersToRange<AddOutputRangeMethods>(classes);
}
