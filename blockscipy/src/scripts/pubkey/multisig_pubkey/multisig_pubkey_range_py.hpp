//
//  multisig_pubkey_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//
//

#ifndef blocksci_multisig_pubkey_range_py_h
#define blocksci_multisig_pubkey_range_py_h

#include <pybind11/pybind11.h>

#include "python_fwd.hpp"

#include <blocksci/scripts/multisig_pubkey_script.hpp>

void addMultisigPubkeyRangeMethods(RangeClasses<blocksci::script::MultisigPubkey> &classes);

#endif /* blocksci_multisig_pubkey_range_py_h */
