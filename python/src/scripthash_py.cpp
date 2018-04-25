//
//  scripthash_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "scripthash_py.hpp"
#include "address_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"
#include "self_apply_py.hpp"

using namespace blocksci;
namespace py = pybind11;

const char *scriptHashDocstring(std::string docstring) {
    return strdup(docstring.c_str());
}

const char *scriptHashRangeDocstring(std::string docstring) {
    std::stringstream ss;
    ss << "For each scripthash: " << docstring;
    return strdup(ss.str().c_str());
}

template <typename T, typename T2>
auto addScriptHashRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    applyMethodsToRange(cl, AddAddressMethods<ScriptBase>{scriptHashRangeDocstring});
    applyMethodsToRange(cl, AddScriptHashBaseMethods<T2>{scriptHashRangeDocstring});
    return cl;
}

template <typename T>
void setupRanges(py::module &m, const std::string &name) {
    std::stringstream ss;
    ss << "Any" << name << "Range";
    addScriptHashRange<ranges::any_view<T>, T>(m, ss.str());
    ss.clear();
    ss << name << "Range";
    addScriptHashRange<ranges::any_view<T, ranges::category::random_access>, T>(m, ss.str());
    ss.clear();
    ss << name << "AnyOptional" << name << "Range";
    addScriptHashRange<ranges::any_view<ranges::optional<T>>, T>(m, ss.str());
    ss.clear();
    ss << name << "Optional" << name << "Range";
    addScriptHashRange<ranges::any_view<ranges::optional<T>, ranges::category::random_access>, T>(m, ss.str());
}

void init_scripthash(py::module &m, py::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::ScriptHash> scriptHashCl(m, "PayToScriptHashAddress", addressCl, "Extra data about pay to script hash address");
    scriptHashCl
    .def("__repr__", &script::ScriptHash::toString)
    .def("__str__", &script::ScriptHash::toPrettyString)
    ;

    applyMethodsToSelf(scriptHashCl, AddScriptHashBaseMethods<script::ScriptHash>{scriptHashDocstring});
    
    py::class_<script::WitnessScriptHash> witnessScriptHashCl(m, "PayToWitnessScriptHashAddress", addressCl, "Extra data about pay to script hash address");
    witnessScriptHashCl
    .def("__repr__", &script::WitnessScriptHash::toString)
    .def("__str__", &script::WitnessScriptHash::toPrettyString)
    ;
    
    applyMethodsToSelf(witnessScriptHashCl, AddScriptHashBaseMethods<script::WitnessScriptHash>{scriptHashDocstring});

    setupRanges<script::ScriptHash>(m, "PayToScriptHashAddress");
    setupRanges<script::WitnessScriptHash>(m, "PayToWitnessScriptHashAddress");
}
