//
//  pubkey_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//
//

#ifndef blocksci_pubkey_range_py_h
#define blocksci_pubkey_range_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/pubkey_script.hpp>

void addPubkeyRangeMethods(RangeClasses<blocksci::script::Pubkey> &classes);
void applyMethodsToPubkeyRange(RangeClasses<blocksci::script::Pubkey> &classes);


#endif /* blocksci_pubkey_range_py_h */
