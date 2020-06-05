//
//  tagged_address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef tagged_address_py_hpp
#define tagged_address_py_hpp

#include "python_fwd.hpp"

#include <blocksci/cluster/cluster_fwd.hpp>

#include <pybind11/pybind11.h>

void init_tagged_address(pybind11::class_<blocksci::TaggedAddress> &cl);
void addTaggedAddressRangeMethods(RangeClasses<blocksci::TaggedAddress> &classes);

#endif /* tagged_address_py_hpp */
