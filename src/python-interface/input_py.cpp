//
//  input_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "input_py.hpp"
#include "optional_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/scripts/script_access.hpp>

#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

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
    return cl;
}

void init_input(py::module &m) {
    py::class_<Input> inputClass(m, "Input", "Class representing a transaction input");
    inputClass
    .def("__repr__", &Input::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Input::getAccess, py::return_value_policy::reference)
    ;
    
    addInputMethods(inputClass, [](auto func) {
        return applyMethodsToSelf<Input>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    
    auto inputRangeClass = addInputRange<ranges::any_view<Input>>(m, "AnyInputRange");
    addInputRangeMethods(inputRangeClass, [](ranges::any_view<Input> &range, auto func) {
        return func(range);
    });
    
    auto inputRangeClass2 = addInputRange<ranges::any_view<Input, ranges::category::random_access>>(m, "InputRange");
    addInputRangeMethods(inputRangeClass2, [](ranges::any_view<Input, ranges::category::random_access> &view, auto func) {
        return func(view);
    });
    
    auto optionalInputRangeClass1 = addInputRange<ranges::any_view<Input>>(m, "AnyOptionalInputRange");
    auto optionalInputRangeClass2 = addInputRange<ranges::any_view<Input, ranges::category::random_access>>(m, "OptionalInputRange");
}
