//
//  tagged_cluster_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "tagged_cluster_range_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/input_range.hpp>
#include <blocksci/chain/output_range.hpp>

namespace py = pybind11;
using namespace blocksci;

void addTaggedClusterRangeMethods(RangeClasses<TaggedCluster> &classes) {
    addAllRangeMethods(classes);
    addTaggedClusterOptionalRangeMethods(classes);
}