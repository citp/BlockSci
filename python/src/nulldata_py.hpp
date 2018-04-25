//
//  nulldata_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nulldata_py_h
#define blocksci_nulldata_py_h

#include <blocksci/scripts/nulldata_script.hpp>
#include <pybind11/pybind11.h>

struct AddOpReturnMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddOpReturnMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
    	using namespace blocksci;
        cl
	    .def_property_readonly("data", func([](const script::OpReturn &address) {
	        return pybind11::bytes(address.getData());
	    }), func2("Data contained inside this address"))
        ;
    }
};

void init_nulldata(pybind11::module &m, pybind11::class_<blocksci::ScriptBase> &addressCl);



#endif /* blocksci_nulldata_py_h */
