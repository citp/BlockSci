//
//  witness_pubkeyhash_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//
//

#ifndef blocksci_witness_pubkeyhash_range_py_h
#define blocksci_witness_pubkeyhash_range_py_h

#include <pybind11/pybind11.h>

#include "python_fwd.hpp"

#include <blocksci/scripts/pubkey_script.hpp>

void addWitnessPubkeyHashRangeMethods(RangeClasses<blocksci::script::WitnessPubkeyHash> &classes);
void addWitnessPubkeyHashOptionalRangeMethods(RangeClasses<blocksci::script::WitnessPubkeyHash> &classes);
void applyMethodsToWitnessPubkeyHashRange(RangeClasses<blocksci::script::WitnessPubkeyHash> &classes);


#endif /* blocksci_witness_pubkeyhash_range_py_h */
