//
//  output_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "output_py.hpp"
#include "ranges_py.hpp"

#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

template <typename T>
auto addOutputRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addOutputMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each output: " << docstring;
        return strdup(ss.str().c_str());
    });
    addOutputRangeMethods(cl, [](auto &range, auto func) {
        return func(range);
    });
    return cl;
}

template <typename T>
auto addOptionalOutputRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addOutputMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each output: " << docstring;
        return strdup(ss.str().c_str());
    });
    addOutputRangeMethods(cl, [](auto &range, auto func) {
        return func(range | flatMapOptionals());
    });
    return cl;
}

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
    
    addOutputRange<ranges::any_view<Output>>(m, "AnyOutputRange");
    addOutputRange<ranges::any_view<Output, ranges::category::random_access>>(m, "OutputRange");
    addOptionalOutputRange<ranges::any_view<ranges::optional<Output>>>(m, "AnyOptionalOutputRange");
    addOptionalOutputRange<ranges::any_view<ranges::optional<Output>, ranges::category::random_access>>(m, "OptionalOutputRange");
}
