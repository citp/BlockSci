//
//  pubkey_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "pubkey_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"
#include "self_apply_py.hpp"

#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>

using namespace blocksci;
namespace py = pybind11;

template <typename T, typename T2>
auto addPubkeyRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addPubkeyBaseMethods<T2>(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each scripthash: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

template <typename T, typename T2>
auto addOptionalPubkeyRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addPubkeyBaseMethods<T2>(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each scripthash: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

template <typename T>
void setupRanges(py::module &m, const std::string &name) {
    std::stringstream ss;
    ss << "Any" << name << "Range";
    addPubkeyRange<ranges::any_view<T>, T>(m, ss.str());
    ss.clear();
    ss << name << "Range";
    addPubkeyRange<ranges::any_view<T, ranges::category::random_access>, T>(m, ss.str());
    ss.clear();
    ss << name << "AnyOptional" << name << "Range";
    addOptionalPubkeyRange<ranges::any_view<ranges::optional<T>>, T>(m, ss.str());
    ss.clear();
    ss << name << "Optional" << name << "Range";
    addOptionalPubkeyRange<ranges::any_view<ranges::optional<T>, ranges::category::random_access>, T>(m, ss.str());
}

void init_pubkey(py::module &m, py::class_<blocksci::ScriptBase> &addressCl) {
	py::class_<script::Pubkey> pubkeyAddressCl(m, "PubkeyAddress", addressCl, "Extra data about pay to pubkey address");
    pubkeyAddressCl
    .def("__repr__", &script::Pubkey::toString)
    .def("__str__", &script::Pubkey::toPrettyString)
    ;

    addPubkeyBaseMethods<script::Pubkey>(pubkeyAddressCl, [](auto func) {
        return applyMethodsToSelf<script::Pubkey>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<script::PubkeyHash> pubkeyHashAddressCl(m, "PubkeyHashAddress", addressCl, "Extra data about pay to pubkey address");
    pubkeyHashAddressCl
    .def("__repr__", &script::PubkeyHash::toString)
    .def("__str__", &script::PubkeyHash::toPrettyString)
    ;

    addPubkeyBaseMethods<script::PubkeyHash>(pubkeyHashAddressCl, [](auto func) {
        return applyMethodsToSelf<script::PubkeyHash>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<script::WitnessPubkeyHash> witnessPubkeyHashAddressCl(m, "WitnessPubkeyHashAddress", addressCl, "Extra data about pay to pubkey address");
    witnessPubkeyHashAddressCl
    .def("__repr__", &script::WitnessPubkeyHash::toString)
    .def("__str__", &script::WitnessPubkeyHash::toPrettyString)
    ;

    addPubkeyBaseMethods<script::WitnessPubkeyHash>(witnessPubkeyHashAddressCl, [](auto func) {
        return applyMethodsToSelf<script::WitnessPubkeyHash>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<script::MultisigPubkey> multisigPubkeyCl(m, "MultisigPubkey", addressCl, "Extra data about a pubkey inside a multisig address");
    multisigPubkeyCl
    .def("__repr__", &script::MultisigPubkey::toString)
    .def("__str__", &script::MultisigPubkey::toPrettyString)
    ;

    addPubkeyBaseMethods<script::MultisigPubkey>(multisigPubkeyCl, [](auto func) {
        return applyMethodsToSelf<script::MultisigPubkey>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });

    setupRanges<script::Pubkey>(m, "PubkeyAddress");
    setupRanges<script::PubkeyHash>(m, "PubkeyHashAddress");
    setupRanges<script::WitnessPubkeyHash>(m, "WitnessPubkeyHashAddress");
    setupRanges<script::MultisigPubkey>(m, "MultisigPubkey");
}
