//
//  multisig_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "multisig_py.hpp"
#include "variant_py.hpp"
#include "ranges_py.hpp"

using namespace blocksci;
namespace py = pybind11;

template <typename T>
auto addMultisigRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addMultisigMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each multisig: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

template <typename T>
auto addOptionalMultisigRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addMultisigMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each multisig: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

void init_multisig(py::module &m, py::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::Multisig> multisigCl(m, "MultisigAddress", addressCl, "Extra data about multi-signature address");
    multisigCl
    .def("__repr__", &script::Multisig::toString)
    .def("__str__", &script::Multisig::toPrettyString)
    ;

    addMultisigMethods(multisigCl, [](auto func) {
        return applyMethodsToSelf<script::Multisig>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });

    addMultisigRange<ranges::any_view<script::Multisig>>(m, "AnyMultisigAddressRange");
    addMultisigRange<ranges::any_view<script::Multisig, ranges::category::random_access>>(m, "MultisigAddressRange");
    addOptionalMultisigRange<ranges::any_view<ranges::optional<script::Multisig>>>(m, "AnyOptionalMultisigAddressRange");
    addOptionalMultisigRange<ranges::any_view<ranges::optional<script::Multisig>, ranges::category::random_access>>(m, "OptionalMultisigAddressRange");
}
