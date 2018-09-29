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

#include <blocksci/chain/block.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/address/equiv_address.hpp>

namespace py = pybind11;
using namespace blocksci;

void applyMethodsToTaggedClusterRange(RangeClasses<TaggedCluster> &classes) {
    applyAllMethodsToRange(classes, AddTaggedClusterMethods{});
}