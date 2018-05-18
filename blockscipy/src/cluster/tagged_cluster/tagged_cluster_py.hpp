//
//  tagged_cluster_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef tagged_cluster_py_hpp
#define tagged_cluster_py_hpp

#include "method_tags.hpp"

#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

void init_tagged_cluster(pybind11::class_<blocksci::TaggedCluster> &cl);

struct AddTaggedClusterMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "cluster", +[](const TaggedCluster &t) { return t.cluster; }, "Return the cluster object which has been tagged");
        func(property_tag, "tagged_addresses", +[](const TaggedCluster &t) { return t.taggedAddresses; }, "Return the list of addresses inside the cluster which have been tagged");
    }
};

#endif /* tagged_cluster_py_hpp */
