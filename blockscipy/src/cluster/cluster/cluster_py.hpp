//
//  cluster_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef cluster_py_hpp
#define cluster_py_hpp

#include "blocksci_range.hpp"

#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

void init_cluster(pybind11::class_<blocksci::Cluster> &cl);
void init_cluster_manager(pybind11::module &s);
void addClusterRangeMethods(RangeClasses<blocksci::Cluster> &classes);

#endif /* cluster_py_hpp */
