//
//  nulldata_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nulldata_py_h
#define blocksci_nulldata_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

#include <pybind11/pybind11.h>

void init_nulldata(pybind11::class_<blocksci::script::OpReturn> &cl);
void addNulldataRangeMethods(RangeClasses<blocksci::script::OpReturn> &classes);

#endif /* blocksci_nulldata_py_h */
