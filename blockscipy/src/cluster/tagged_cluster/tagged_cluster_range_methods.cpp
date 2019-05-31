//
//  tagged_cluster_range_methods.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "tagged_cluster_range_py.hpp"
#include "tagged_cluster_py.hpp"
#include "range_apply_py.hpp"
#include "caster_py.hpp"

namespace py = pybind11;
using namespace blocksci;

void applyMethodsToTaggedClusterRange(RangeClasses<TaggedCluster> &classes) {
    applyMethodsToRange(classes, AddTaggedClusterMethods{});
}