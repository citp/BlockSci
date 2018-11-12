//
//  pubkey_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_pubkey_py_h
#define blocksci_pubkey_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

#include <pybind11/pybind11.h>

void init_pubkey(pybind11::class_<blocksci::script::Pubkey> &cl);
void init_pubkeyhash(pybind11::class_<blocksci::script::PubkeyHash> &cl);
void init_witness_pubkeyhash(pybind11::class_<blocksci::script::WitnessPubkeyHash> &cl);
void init_multisig_pubkey(pybind11::class_<blocksci::script::MultisigPubkey> &cl);

void addMultisigPubkeyRangeMethods(RangeClasses<blocksci::script::MultisigPubkey> &classes);
void addPubkeyRangeMethods(RangeClasses<blocksci::script::Pubkey> &classes);
void addPubkeyHashRangeMethods(RangeClasses<blocksci::script::PubkeyHash> &classes);
void addWitnessPubkeyHashRangeMethods(RangeClasses<blocksci::script::WitnessPubkeyHash> &classes);

#endif /* blocksci_pubkey_py_h */
