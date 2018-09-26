//
//  scripthash_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_scripthash_range_py_h
#define blocksci_scripthash_range_py_h

#include <pybind11/pybind11.h>

#include "python_fwd.hpp"

#include <blocksci/scripts/scripthash_script.hpp>

void addScriptHashRangeMethods(RangeClasses<blocksci::script::ScriptHash> &classes);
void addScripthashOptionalRangeMethods(RangeClasses<blocksci::script::ScriptHash> &classes);
void applyMethodsToScriptHashRange(RangeClasses<blocksci::script::ScriptHash> &classes);


#endif /* blocksci_scripthash_range_py_h */
