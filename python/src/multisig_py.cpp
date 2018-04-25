//
//  multisig_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "multisig_py.hpp"
#include "address_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"
#include "self_apply_py.hpp"

using namespace blocksci;
namespace py = pybind11;

pybind11::list pyMultisigAddresses(blocksci::script::Multisig &script) {
    pybind11::list ret;
    for (auto &address : script.pubkeyScripts()) {
        ret.append(address);
    }
    return ret;
}

const char *multisigRangeDocstring(std::string docstring) {
    std::stringstream ss;
    ss << "For each address: " << docstring;
    return strdup(ss.str().c_str());
}

const char *multisigDocstring(std::string docstring) {
    return strdup(docstring.c_str());
}

template <typename T>
auto addMultisigRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    applyMethodsToRange(cl, AddAddressMethods<ScriptBase>{multisigRangeDocstring});
    applyMethodsToRange(cl, AddMultisigMethods{multisigRangeDocstring});
    return cl;
}

void init_multisig(py::module &m, py::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::Multisig> multisigCl(m, "MultisigAddress", addressCl, "Extra data about multi-signature address");
    multisigCl
    .def("__repr__", &script::Multisig::toString)
    .def("__str__", &script::Multisig::toPrettyString)
    ;

    applyMethodsToSelf(multisigCl, AddMultisigMethods{multisigDocstring});

    addMultisigRange<ranges::any_view<script::Multisig>>(m, "AnyMultisigAddressRange");
    addMultisigRange<ranges::any_view<script::Multisig, ranges::category::random_access>>(m, "MultisigAddressRange");
    addMultisigRange<ranges::any_view<ranges::optional<script::Multisig>>>(m, "AnyOptionalMultisigAddressRange");
    addMultisigRange<ranges::any_view<ranges::optional<script::Multisig>, ranges::category::random_access>>(m, "OptionalMultisigAddressRange");
}
