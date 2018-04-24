//
//  pubkey_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_pubkey_py_h
#define blocksci_pubkey_py_h

#include <blocksci/scripts/multisig_script.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/utility/optional.hpp>

template <typename T, typename Class, typename FuncApplication, typename FuncDoc>
void addPubkeyBaseMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def("find_multisigs", func([](const T &script) {
        pybind11::list ret;
        for (auto &address : script.getIncludingMultisigs()) {
            ret.append(script::Multisig{address.scriptNum, script.getAccess()});
        }
        return ret;
    }), func2("List of multisigs which include this public key"))
    .def_property_readonly("pubkey", func([](const T &script) -> ranges::optional<pybind11::bytes> {
        auto pubkey = script.getPubkey();
        if (pubkey) {
            return pybind11::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
        } else {
            return ranges::nullopt;
        }
    }), func2("Public key for this address"))
    .def_property_readonly("pubkeyhash", func([](const T &address) {
        return address.getPubkeyHash();
    }), func2("160 bit address hash"))
    .def_property_readonly("address_string", func([](const T &address) {
        return address.addressString();
    }), func2("Bitcoin address string"))
    ;
}

void init_pubkey(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_pubkey_py_h */
