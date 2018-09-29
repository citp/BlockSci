//
//  tagged_cluster_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef tagged_cluster_range_py_hpp
#define tagged_cluster_range_py_hpp

#include "python_fwd.hpp"

#include <blocksci/cluster/cluster.hpp>

void addTaggedClusterRangeMethods(RangeClasses<blocksci::TaggedCluster> &classes);
void applyMethodsToTaggedClusterRange(RangeClasses<blocksci::TaggedCluster> &classes);

#endif /* tagged_cluster_range_py_hpp */
