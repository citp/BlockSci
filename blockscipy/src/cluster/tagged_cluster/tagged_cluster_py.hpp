//
//  tagged_cluster_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef tagged_cluster_py_hpp
#define tagged_cluster_py_hpp

#include "python_fwd.hpp"

#include <blocksci/cluster/cluster_fwd.hpp>

#include <pybind11/pybind11.h>

void init_tagged_cluster(pybind11::class_<blocksci::TaggedCluster> &cl);
void addTaggedClusterRangeMethods(RangeClasses<blocksci::TaggedCluster> &classes);

#endif /* tagged_cluster_py_hpp */
