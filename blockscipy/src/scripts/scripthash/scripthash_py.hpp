//
//  scripthash_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_scripthash_py_h
#define blocksci_scripthash_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

#include <pybind11/pybind11.h>

void init_scripthash(pybind11::class_<blocksci::script::ScriptHash> &cl);
void init_witness_scripthash(pybind11::class_<blocksci::script::WitnessScriptHash> &cl);

void addScriptHashRangeMethods(RangeClasses<blocksci::script::ScriptHash> &classes);
void addWitnessScriptHashRangeMethods(RangeClasses<blocksci::script::WitnessScriptHash> &classes);

#endif /* blocksci_scripthash_py_h */
