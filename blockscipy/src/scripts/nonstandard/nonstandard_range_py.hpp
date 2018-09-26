//
//  nonstandard_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nonstandard_range_py_h
#define blocksci_nonstandard_range_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/multisig_script.hpp>

void addNonstandardRangeMethods(RangeClasses<blocksci::script::Nonstandard> &classes);
void addNonstandardOptionalRangeMethods(RangeClasses<blocksci::script::Nonstandard> &classes);
void applyMethodsToNonstandardRange(RangeClasses<blocksci::script::Nonstandard> &classes);

#endif /* blocksci_nonstandard_range_py_h */
