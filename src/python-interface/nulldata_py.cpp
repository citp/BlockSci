//
//  nulldata_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "nulldata_py.hpp"
#include "variant_py.hpp"
#include "ranges_py.hpp"

using namespace blocksci;
namespace py = pybind11;

template <typename T>
auto addNulldataRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addOpReturnMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each multisig: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

template <typename T>
auto addOptionalNulldataRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addOpReturnMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each multisig: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}
void init_nulldata(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::OpReturn> opReturnCl(m, "OpReturnAddress", addressCl, "Extra data about op_return address");
    opReturnCl
    .def("__repr__", &script::OpReturn::toString)
    .def("__str__", &script::OpReturn::toPrettyString)
    ;

    addOpReturnMethods(opReturnCl, [](auto func) {
        return applyMethodsToSelf<script::OpReturn>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });

    addNulldataRange<ranges::any_view<script::OpReturn>>(m, "AnyOpReturnAddressRange");
    addNulldataRange<ranges::any_view<script::OpReturn, ranges::category::random_access>>(m, "OpReturnAddressRange");
    addOptionalNulldataRange<ranges::any_view<ranges::optional<script::OpReturn>>>(m, "AnyOptionalOpReturnAddressRange");
    addOptionalNulldataRange<ranges::any_view<ranges::optional<script::OpReturn>, ranges::category::random_access>>(m, "OptionalOpReturnAddressRange");
}

