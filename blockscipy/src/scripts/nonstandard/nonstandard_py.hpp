//
//  nonstandard_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nonstandard_py_h
#define blocksci_nonstandard_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

#include <pybind11/pybind11.h>

void init_nonstandard(pybind11::class_<blocksci::script::Nonstandard> &cl);
void addNonstandardRangeMethods(RangeClasses<blocksci::script::Nonstandard> &classes);

#endif /* blocksci_nonstandard_py_h */
