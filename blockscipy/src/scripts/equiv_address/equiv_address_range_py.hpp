//
//  equiv_address_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef equiv_address_range_py_h
#define equiv_address_range_py_h

#include "python_fwd.hpp"

#include <blocksci/address/equiv_address.hpp>

void addEquivAddressRangeMethods(RangeClasses<blocksci::EquivAddress> &classes);
void addEquivAddressOptionalRangeMethods(RangeClasses<blocksci::EquivAddress> &classes);
void applyMethodsToEquivAddressRange(RangeClasses<blocksci::EquivAddress> &classes);

#endif /* equiv_address_range_py_h */
