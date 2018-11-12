//
//  tagged_cluster_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "tagged_cluster_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/chain/input_range.hpp>
#include <blocksci/chain/output_range.hpp>
#include <blocksci/cluster/cluster.hpp>

namespace py = pybind11;
using namespace blocksci;

void init_tagged_cluster(py::class_<TaggedCluster> &) {
	
}

void addTaggedClusterRangeMethods(RangeClasses<TaggedCluster> &classes) {
    addAllRangeMethods(classes);
}
