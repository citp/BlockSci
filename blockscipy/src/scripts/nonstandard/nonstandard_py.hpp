//
//  nonstandard_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nonstandard_py_h
#define blocksci_nonstandard_py_h

#include "method_tags.hpp"

#include <blocksci/scripts/nonstandard_script.hpp>
#include <pybind11/pybind11.h>

struct AddNonstandardMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
    	using namespace blocksci;
        func(property_tag, "in_script", &blocksci::script::Nonstandard::inputString, "Nonstandard input script");
        func(property_tag, "out_script", &blocksci::script::Nonstandard::outputString, "Nonstandard output script");
    }
};

void init_nonstandard(pybind11::class_<blocksci::script::Nonstandard> &cl);



#endif /* blocksci_nonstandard_py_h */
