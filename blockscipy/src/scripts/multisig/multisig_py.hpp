//
//  multisig_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_multisig_py_h
#define blocksci_multisig_py_h

#include "blocksci_range.hpp"

#include <blocksci/scripts/multisig_script.hpp>

#include <pybind11/pybind11.h>

void init_multisig(pybind11::class_<blocksci::script::Multisig> &cl);
void addMultisigRangeMethods(RangeClasses<blocksci::script::Multisig> &classes);

#endif /* blocksci_multisig_py_h */
