//
//  tagged_address_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef tagged_address_range_py_hpp
#define tagged_address_range_py_hpp

#include "python_fwd.hpp"

#include <blocksci/cluster/cluster.hpp>

void addTaggedAddressRangeMethods(RangeClasses<blocksci::TaggedAddress> &classes);
void addTaggedAddressOptionalRangeMethods(RangeClasses<blocksci::TaggedAddress> &classes);
void applyMethodsToTaggedAddressRange(RangeClasses<blocksci::TaggedAddress> &classes);

#endif /* tagged_address_range_py_hpp */
