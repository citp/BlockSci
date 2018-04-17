//
//  multisig_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_multisig_py_h
#define blocksci_multisig_py_h

#include "any_script_caster.hpp"
#include "optional_py.hpp"

#include <blocksci/scripts/multisig_script.hpp>
#include <pybind11/pybind11.h>

template <typename Class, typename FuncApplication, typename FuncDoc>
void addMultisigMethods(Class &cl, FuncApplication func, FuncDoc func2) {
	using namespace blocksci;
    cl
    .def_property_readonly("required", func([](const script::Multisig &address) -> int64_t {
    	return address.getRequired();
    }), func2("The number of signatures required for this address"))
    .def_property_readonly("total", func([](const script::Multisig &address) -> int64_t {
    	return address.getTotal();
    }), func2("The total number of keys that can sign for this address"))
    .def_property_readonly("addresses", func([](const script::Multisig &script) {
        pybind11::list ret;
        for (auto &address : script.pubkeyScripts()) {
            ret.append(address);
        }
        return ret;
    }), func2("The list of the keys that can sign for this address"))
    ;
}

void init_multisig(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_multisig_py_h */
