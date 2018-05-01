//
//  scripthash_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_scripthash_py_h
#define blocksci_scripthash_py_h

#include "method_tags.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

#include <pybind11/pybind11.h>

template <typename T>
struct AddScriptHashBaseMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(property_tag, "wrapped_address", &T::wrappedScript, "The address inside this P2SH address");
        func(property_tag, "raw_address",  &T::getAddressHash, "The 160 bit P2SH address hash");
        func(property_tag, "address_string", &T::addressString, "Bitcoin address string");
    }
};

void init_scripthash(pybind11::class_<blocksci::script::ScriptHash> &cl);
void init_witness_scripthash(pybind11::class_<blocksci::script::WitnessScriptHash> &cl);


#endif /* blocksci_scripthash_py_h */
