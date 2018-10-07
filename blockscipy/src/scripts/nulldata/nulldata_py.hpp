//
//  nulldata_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nulldata_py_h
#define blocksci_nulldata_py_h

#include "method_tags.hpp"
#include "blocksci_range.hpp"

#include <blocksci/scripts/nulldata_script.hpp>
#include <pybind11/pybind11.h>

struct AddOpReturnMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
    	using namespace blocksci;
	    func(property_tag, "data", +[](const script::OpReturn &address) {
	        return pybind11::bytes(address.getData());
	    }, "Data contained inside this address");
    }
};

void init_nulldata(pybind11::class_<blocksci::script::OpReturn> &cl);
void addNulldataRangeMethods(RangeClasses<blocksci::script::OpReturn> &classes);

#endif /* blocksci_nulldata_py_h */
