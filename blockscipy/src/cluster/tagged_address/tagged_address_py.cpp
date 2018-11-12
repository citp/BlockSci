//
//  tagged_address_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "tagged_address_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/cluster/cluster.hpp>

namespace py = pybind11;
using namespace blocksci;

void init_tagged_address(py::class_<TaggedAddress> &) {

}

void addTaggedAddressRangeMethods(RangeClasses<TaggedAddress> &classes) {
    addAllRangeMethods(classes);
}
