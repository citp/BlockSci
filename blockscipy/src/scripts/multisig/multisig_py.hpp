//
//  multisig_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_multisig_py_h
#define blocksci_multisig_py_h

#include "method_tags.hpp"

#include <blocksci/scripts/multisig_script.hpp>

#include <pybind11/pybind11.h>

pybind11::list pyMultisigAddresses(blocksci::script::Multisig &script);

struct AddMultisigMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(property_tag, "required", &blocksci::script::Multisig::getRequired, "The number of signatures required for this address");
		func(property_tag, "total", &blocksci::script::Multisig::getTotal, "The total number of keys that can sign for this address");
		func(property_tag, "addresses", pyMultisigAddresses, "The list of the keys that can sign for this address");
    }
};

void init_multisig(pybind11::class_<blocksci::script::Multisig> &cl);

#endif /* blocksci_multisig_py_h */
