//
//  witness_unknown_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/22/18.
//
//

#ifndef blocksci_witness_unknown_py_h
#define blocksci_witness_unknown_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

#include <pybind11/pybind11.h>

void init_witness_unknown(pybind11::class_<blocksci::script::WitnessUnknown> &cl);
void addWitnessUnknownRangeMethods(RangeClasses<blocksci::script::WitnessUnknown> &classes);

#endif /* blocksci_witness_unknown_py_h */
