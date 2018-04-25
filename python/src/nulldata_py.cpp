//
//  nulldata_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "nulldata_py.hpp"
#include "address_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"
#include "self_apply_py.hpp"

using namespace blocksci;
namespace py = pybind11;

const char *opReturndRangeDocstring(std::string docstring) {
    std::stringstream ss;
    ss << "For each op_return: " << docstring;
    return strdup(ss.str().c_str());
}

const char *opReturnDocstring(std::string docstring) {
    return strdup(docstring.c_str());
}

template <typename T>
auto addNulldataRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    applyMethodsToRange(cl, AddAddressMethods<ScriptBase>{opReturndRangeDocstring});
    applyMethodsToRange(cl, AddOpReturnMethods{opReturndRangeDocstring});
    return cl;
}

void init_nulldata(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::OpReturn> opReturnCl(m, "OpReturnAddress", addressCl, "Extra data about op_return address");
    opReturnCl
    .def("__repr__", &script::OpReturn::toString)
    .def("__str__", &script::OpReturn::toPrettyString)
    ;

    applyMethodsToSelf(opReturnCl, AddOpReturnMethods{opReturnDocstring});
    
    addNulldataRange<ranges::any_view<script::OpReturn>>(m, "AnyOpReturnAddressRange");
    addNulldataRange<ranges::any_view<script::OpReturn, ranges::category::random_access>>(m, "OpReturnAddressRange");
    addNulldataRange<ranges::any_view<ranges::optional<script::OpReturn>>>(m, "AnyOptionalOpReturnAddressRange");
    addNulldataRange<ranges::any_view<ranges::optional<script::OpReturn>, ranges::category::random_access>>(m, "OptionalOpReturnAddressRange");
}

