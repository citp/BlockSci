//
//  scripthash_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_scripthash_py_h
#define blocksci_scripthash_py_h

#include <blocksci/scripts/script_variant.hpp>
#include <pybind11/pybind11.h>

template <typename T>
struct AddScriptHashBaseMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddScriptHashBaseMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        using namespace blocksci;
        cl
        .def_property_readonly("wrapped_address", func(&T::wrappedScript), func2("The address inside this P2SH address"))
        .def_property_readonly("raw_address",  func(&T::getAddressHash), func2("The 160 bit P2SH address hash"))
        .def_property_readonly("address_string", func(&T::addressString), func2("Bitcoin address string"))
        ;
    }
};

void init_scripthash(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_scripthash_py_h */
