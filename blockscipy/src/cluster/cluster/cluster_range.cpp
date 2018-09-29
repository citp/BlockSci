//
//  cluster_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "cluster_range_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/input_range.hpp>
#include <blocksci/chain/output_range.hpp>
#include <blocksci/address/equiv_address.hpp>

namespace py = pybind11;
using namespace blocksci;

void addClusterRangeMethods(RangeClasses<Cluster> &classes) {
    addAllRangeMethods(classes);
}