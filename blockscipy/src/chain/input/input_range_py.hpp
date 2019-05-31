//
//  input_range_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef input_range_py_hpp
#define input_range_py_hpp

#include "python_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

void addInputRangeMethods(RangeClasses<blocksci::Input> &classes);
void applyMethodsToInputRange(RangeClasses<blocksci::Input> &classes);
void applyRangeFiltersToInputRange(RangeClasses<blocksci::Input> &classes);

#endif /* input_range_py_hpp */
