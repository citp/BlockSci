//
//  address_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef address_range_py_h
#define address_range_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addAddressRangeMethods(RangeClasses<blocksci::AnyScript> &classes);
void addAddressOptionalRangeMethods(RangeClasses<blocksci::AnyScript> &classes);
void applyMethodsToAddressRange(RangeClasses<blocksci::AnyScript> &classes);
void applyRangeFiltersToAddressRange(RangeClasses<blocksci::AnyScript> &classes);

#endif /* address_range_py_h */
