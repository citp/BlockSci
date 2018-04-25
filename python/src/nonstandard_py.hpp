//
//  nonstandard_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nonstandard_py_h
#define blocksci_nonstandard_py_h

#include <blocksci/scripts/nonstandard_script.hpp>
#include <pybind11/pybind11.h>

struct AddNonstandardMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddNonstandardMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
    	using namespace blocksci;
        cl
        .def_property_readonly("in_script", func(&blocksci::script::Nonstandard::inputString), func2("Nonstandard input script"))
		.def_property_readonly("total", func(&blocksci::script::Nonstandard::outputString), func2("Nonstandard output script"))
        ;
    }
};

void init_nonstandard(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_nonstandard_py_h */
