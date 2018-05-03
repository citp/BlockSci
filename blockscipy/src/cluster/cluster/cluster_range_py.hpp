//
//  cluster_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef cluster_range_py_hpp
#define cluster_range_py_hpp

#include "python_fwd.hpp"

#include <blocksci/cluster/cluster.hpp>

void addClusterRangeMethods(RangeClasses<blocksci::Cluster> &classes);
void applyMethodsToClusterRange(RangeClasses<blocksci::Cluster> &classes);

#endif /* cluster_range_py_hpp */
