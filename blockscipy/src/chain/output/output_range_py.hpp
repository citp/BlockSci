//
//  output_range_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef output_range_py_hpp
#define output_range_py_hpp

#include "python_fwd.hpp"

#include <blocksci/chain/output.hpp>

void addOutputRangeMethods(RangeClasses<blocksci::Output> &classes);
void applyMethodsToOutputRange(RangeClasses<blocksci::Output> &classes);
void applyRangeFiltersToOutputRange(RangeClasses<blocksci::Output> &classes);

#endif /* output_range_py_hpp */
