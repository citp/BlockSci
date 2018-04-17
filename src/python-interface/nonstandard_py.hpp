//
//  nonstandard_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nonstandard_py_h
#define blocksci_nonstandard_py_h

#include "any_script_caster.hpp"
#include "optional_py.hpp"

#include <blocksci/scripts/nonstandard_script.hpp>
#include <pybind11/pybind11.h>

template <typename Class, typename FuncApplication, typename FuncDoc>
void addNonstandardMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("in_script", func([](const script::Nonstandard &address) {
        return address.inputString();
    }), func2("Nonstandard input script"))
    .def_property_readonly("out_script", func([](const script::Nonstandard &address) {
        return address.outputString();
    }), func2("Nonstandard output script"))
    ;
}

void init_nonstandard(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_nonstandard_py_h */
