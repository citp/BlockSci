//
//  output_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "output_py.hpp"
#include "optional_py.hpp"
#include "ranges_py.hpp"
#include "variant_py.hpp"

#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/scripts/script_access.hpp>

#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_output(py::module &m) {
    py::class_<Output> outputClass(m, "Output", "Class representing a transaction output");
    outputClass
    .def("__repr__", &Output::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Output::getAccess, py::return_value_policy::reference)
    ;
    
    addOutputMethods(outputClass, [](auto func) {
        return applyMethodsToSelf<Output>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    auto outputRangeClass = addRangeClass<ranges::any_view<Output>>(m, "AnyOutputRange");
    addOutputMethods(outputRangeClass, [](auto func) {
        return applyMethodsToRange<ranges::any_view<Output>>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each output: " << docstring;
        return strdup(ss.str().c_str());
    });
    addOutputRangeMethods(outputRangeClass, [](auto &&range, auto func) {
        return func(range);
    });
    
    auto outputRangeClass2 = addRangeClass<ranges::any_view<Output, ranges::category::random_access>>(m, "OutputRange");
    addOutputMethods(outputRangeClass2, [](auto func) {
        return applyMethodsToRange<ranges::any_view<Output, ranges::category::random_access>>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each output: " << docstring;
        return strdup(ss.str().c_str());
    });
    addOutputRangeMethods(outputRangeClass2, [](ranges::any_view<Output, ranges::category::random_access> &range, auto func) {
        return func(range);
    });
    
    addRangeClass<ranges::any_view<ranges::optional<Output>>>(m, "OptionalOutputRange");
}
