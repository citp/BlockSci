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
    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        cl
        .def_property_readonly("unspent", func([](Range & range) {
            return outputsUnspent(range);
        }), "Returns a range including the subset of outputs which were never spent")
        .def("spent_before",  func([](Range &range, BlockHeight height) {
            return outputsSpentBeforeHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent before the given height")
        .def("spent_after",  func([](Range &range, BlockHeight height) {
            return outputsSpentAfterHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent after the given height")
        .def("spent_within",  func([](Range &range, BlockHeight height) {
            return outputsSpentWithinRelativeHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent within the given number of blocks")
        .def("spent_outside",  func([](Range &range, BlockHeight height) {
            return outputsSpentOutsideRelativeHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent later than the given number of blocks")
        .def("with_type", func([](Range &range, AddressType::Enum type) {
            return outputsOfType(range, type);
        }), "Returns a range including the subset of outputs which were sent to the given address type")
        ;
    }
};

void applyRangeFiltersToOutputRange(RangeClasses<Output> &classes) {
    applyRangeFiltersToRange<AddOutputRangeMethods>(classes);
}
