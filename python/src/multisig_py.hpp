//
//  multisig_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_multisig_py_h
#define blocksci_multisig_py_h

#include <blocksci/scripts/multisig_script.hpp>
#include <pybind11/pybind11.h>

pybind11::list pyMultisigAddresses(blocksci::script::Multisig &script);

struct AddMultisigMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddMultisigMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        cl
        .def_property_readonly("required", func(&blocksci::script::Multisig::getRequired), func2("The number of signatures required for this address"))
		.def_property_readonly("total", func(&blocksci::script::Multisig::getTotal), func2("The total number of keys that can sign for this address"))
		.def_property_readonly("addresses", func(pyMultisigAddresses), func2("The list of the keys that can sign for this address"))
        ;
    }
};

void init_multisig(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_multisig_py_h */
