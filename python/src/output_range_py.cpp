//
//  output_range_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "output_range_py.hpp"
#include "output_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"

namespace py = pybind11;

using namespace blocksci;

template <typename Range, typename Class, typename FuncApplication>
struct AddOutputRangeMethods {
    void operator()(Class &cl, FuncApplication func) {
        cl
        .def_property_readonly("unspent", func([=](Range && range) {
            return outputsUnspent(range);
        }), "Returns a range including the subset of outputs which were never spent")
        .def("spent_before",  func([](Range && range, BlockHeight height) {
            return outputsSpentBeforeHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent before the given height")
        .def("spent_after",  func([](Range && range, BlockHeight height) {
            return outputsSpentAfterHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent after the given height")
        .def("spent_within",  func([](Range && range, BlockHeight height) {
            return outputsSpentWithinRelativeHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent within the given number of blocks")
        .def("spent_outside",  func([](Range && range, BlockHeight height) {
            return outputsSpentOutsideRelativeHeight(range, height);
        }), "Returns a range including the subset of outputs which were spent later than the given number of blocks")
        .def("with_type", func([](Range && range, AddressType::Enum type) {
            return outputsOfType(range, type);
        }), "Returns a range including the subset of outputs which were sent to the given address type")
        ;
    }
};

const char *outputRangeDocstring(std::string docstring) {
    std::stringstream ss;
    ss << "For each output: " << docstring;
    return strdup(ss.str().c_str());
}

template <typename T>
auto addOutputRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addOutputMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, outputRangeDocstring);
    applyRangeMethodsToRange<AddOutputRangeMethods>(cl);
    return cl;
}

template <typename T>
auto addOptionalOutputRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addOutputMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, outputRangeDocstring);
    applyRangeMethodsToRange<AddOutputRangeMethods>(cl);
    return cl;
}

namespace py = pybind11;

using namespace blocksci;

void init_output_range(py::module &m) {    
    addOutputRange<ranges::any_view<Output>>(m, "AnyOutputRange");
    addOutputRange<ranges::any_view<Output, ranges::category::random_access>>(m, "OutputRange");
    addOptionalOutputRange<ranges::any_view<ranges::optional<Output>>>(m, "AnyOptionalOutputRange");
    addOptionalOutputRange<ranges::any_view<ranges::optional<Output>, ranges::category::random_access>>(m, "OptionalOutputRange");
}
