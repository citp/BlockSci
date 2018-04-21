//
//  nonstandard_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "nonstandard_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"
#include "self_apply_py.hpp"

using namespace blocksci;
namespace py = pybind11;

template <typename T>
auto addNonstandardRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addNonstandardMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each multisig: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

template <typename T>
auto addOptionalNonstandardRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addNonstandardMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each multisig: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

void init_nonstandard(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::Nonstandard> nonstandardCl(m, "NonStandardAddress", addressCl, "Extra data about non-standard address");
    nonstandardCl
    .def("__repr__", &script::Nonstandard::toString)
    .def("__str__", &script::Nonstandard::toPrettyString)
    ;

    addNonstandardMethods(nonstandardCl, [](auto func) {
        return applyMethodsToSelf<script::Nonstandard>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });

    addNonstandardRange<ranges::any_view<script::Nonstandard>>(m, "AnyNonStandardAddressRange");
    addNonstandardRange<ranges::any_view<script::Nonstandard, ranges::category::random_access>>(m, "NonStandardAddressRange");
    addOptionalNonstandardRange<ranges::any_view<ranges::optional<script::Nonstandard>>>(m, "AnyOptionalNonStandardAddressRange");
    addOptionalNonstandardRange<ranges::any_view<ranges::optional<script::Nonstandard>, ranges::category::random_access>>(m, "OptionalNonStandardAddressRange");
}

