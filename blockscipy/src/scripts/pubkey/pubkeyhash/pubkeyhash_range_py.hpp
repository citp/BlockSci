//
//  pubkeyhash_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//
//

#ifndef blocksci_pubkeyhash_range_py_h
#define blocksci_pubkeyhash_range_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/pubkey_script.hpp>

#include <pybind11/pybind11.h>

void addPubkeyHashRangeMethods(RangeClasses<blocksci::script::PubkeyHash> &classes);

#endif /* blocksci_pubkeyhash_range_py_h */
