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

template <typename Range, typename Class, typename FuncApplication>
struct AddInputRangeMethods {
    void operator()(Class &cl, FuncApplication func) {
        cl
        .def("sent_before",  func([](Range &range, BlockHeight height) {
            return inputsCreatedBeforeHeight(r, height);
        }), "Returns a range including the subset of inputs which spent an output created before the given height")
        .def("sent_after",  func([](Range &range, blocksci::BlockHeight height) {
            return inputsCreatedAfterHeight(range, height);
        }), "Returns a range including the subset of inputs which spent an output created after the given height")
        .def("sent_within",  func([](Range &range, blocksci::BlockHeight height) {
            return inputsCreatedWithinRelativeHeight(range, height);
        }), "Returns a range including the subset of inputs which spent an output created more than a given number of blocks before the input")
        .def("sent_outside",  func([](Range &range, blocksci::BlockHeight height) {
            return inputsCreatedOutsideRelativeHeight(range, height);
        }), "Returns a range including the subset of inputs which spent an output created less than a given number of blocks before the input")
        .def("with_type", func([](Range &range, AddressType::Enum type) {
            return inputsOfType(range, type);
        }), "Return a range including only inputs sent to the given address type")
        ;
    }
};


template <typename T>
auto addInputRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addInputMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each input: " << docstring;
        return strdup(ss.str().c_str());
    });
    applyRangeMethodsToRange<AddInputRangeMethods>(cl);
    return cl;
}

template <typename T>
auto addOptionalInputRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addInputMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each input: " << docstring;
        return strdup(ss.str().c_str());
    });
    applyRangeMethodsToRange<AddInputRangeMethods>(cl);
    return cl;
}

void init_input_range(pybind11::module &m) {
    addInputRange<ranges::any_view<Input>>(m, "AnyInputRange");
    addInputRange<ranges::any_view<Input, ranges::category::random_access>>(m, "InputRange");
    addOptionalInputRange<ranges::any_view<ranges::optional<Input>>>(m, "AnyOptionalInputRange");
    addOptionalInputRange<ranges::any_view<ranges::optional<Input>, ranges::category::random_access>>(m, "OptionalInputRange");
}
