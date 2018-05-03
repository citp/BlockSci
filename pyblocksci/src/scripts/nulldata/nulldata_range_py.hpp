//
//  nulldata_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_nulldata_range_py_h
#define blocksci_nulldata_range_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/nulldata_script.hpp>

void addNulldataRangeMethods(RangeClasses<blocksci::script::OpReturn> &classes);
void applyMethodsToNulldataRange(RangeClasses<blocksci::script::OpReturn> &classes);


#endif /* blocksci_nulldata_range_py_h */
