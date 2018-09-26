//
//  witness_scripthash_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_witness_scripthash_range_py_h
#define blocksci_witness_scripthash_range_py_h

#include <pybind11/pybind11.h>

#include "python_fwd.hpp"

#include <blocksci/scripts/scripthash_script.hpp>

void addWitnessScriptHashRangeMethods(RangeClasses<blocksci::script::WitnessScriptHash> &classes);
void addWitnessScriptHashOptionalRangeMethods(RangeClasses<blocksci::script::WitnessScriptHash> &classes);
void applyMethodsToWitnessScriptHashRange(RangeClasses<blocksci::script::WitnessScriptHash> &classes);


#endif /* blocksci_witness_scripthash_range_py_h */
