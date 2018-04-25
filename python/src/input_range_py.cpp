//
//  input_range_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "input_range_py.hpp"
#include "input_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"

namespace py = pybind11;
using namespace blocksci;

template <typename Range>
struct AddInputRangeMethods {
    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        cl
        .def("sent_before",  func([](Range &&range, BlockHeight height) {
            return inputsCreatedBeforeHeight(range, height);
        }), "Returns a range including the subset of inputs which spent an output created before the given height")
        .def("sent_after",  func([](Range &&range, BlockHeight height) {
            return inputsCreatedAfterHeight(range, height);
        }), "Returns a range including the subset of inputs which spent an output created after the given height")
        .def("sent_within",  func([](Range &&range, BlockHeight height) {
            return inputsCreatedWithinRelativeHeight(range, height);
        }), "Returns a range including the subset of inputs which spent an output created more than a given number of blocks before the input")
        .def("sent_outside",  func([](Range &&range, BlockHeight height) {
            return inputsCreatedOutsideRelativeHeight(range, height);
        }), "Returns a range including the subset of inputs which spent an output created less than a given number of blocks before the input")
        .def("with_type", func([](Range && range, AddressType::Enum type) {
            return inputsOfType(range, type);
        }), "Return a range including only inputs sent to the given address type")
        ;
    }
};


const char *inputRangeDocstring(std::string docstring) {
    std::stringstream ss;
    ss << "For each input: " << docstring;
    return strdup(ss.str().c_str());
}

void init_input_range(pybind11::module &m) {
    {
        auto cl = addRangeClass<ranges::any_view<Input>>(m, "AnyInputRange");
        applyMethodsToRange(cl, AddInputMethods{inputRangeDocstring});
        applyRangeMethodsToRange(cl, AddInputRangeMethods<ranges::any_view<Input>>{});
    }

    {
        auto cl = addRangeClass<ranges::any_view<Input, ranges::category::random_access>>(m, "InputRange");
        applyMethodsToRange(cl, AddInputMethods{inputRangeDocstring});
        applyRangeMethodsToRange(cl, AddInputRangeMethods<ranges::any_view<Input, ranges::category::random_access>>{});
    }

    {
        auto cl = addRangeClass<ranges::any_view<ranges::optional<Input>>>(m, "AnyOptionalInputRange");
        applyMethodsToRange(cl, AddInputMethods{inputRangeDocstring});
    }

    {
        auto cl = addRangeClass<ranges::any_view<ranges::optional<Input>, ranges::category::random_access>>(m, "OptionalInputRange");
        applyMethodsToRange(cl, AddInputMethods{inputRangeDocstring});
    }
}
