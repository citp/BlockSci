//
//  scripthash_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_scripthash_py_h
#define blocksci_scripthash_py_h

#include "any_script_caster.hpp"
#include <blocksci/scripts/script_variant.hpp>
#include <pybind11/pybind11.h>

template <typename T, typename Class, typename FuncApplication, typename FuncDoc>
void addScriptHashMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("wrapped_address", func([](const T &script) -> ranges::optional<AnyScript> {
        auto wrappedScript = script.wrappedScript();
        if (wrappedScript) {
            return wrappedScript;
        } else {
            return ranges::nullopt;
        }
    }), func2("The address inside this P2SH address"))
    .def_property_readonly("raw_address",  func([](const T &address) {
        return address.getAddressHash();
    }), func2("The 160 bit P2SH address hash"))
    .def_property_readonly("address_string", func([](const T &address) {
        return address.addressString();
    }), func2("Bitcoin address string"))
    ;
}

void init_scripthash(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_scripthash_py_h */