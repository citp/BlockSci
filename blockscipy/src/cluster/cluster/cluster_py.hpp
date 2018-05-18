//
//  cluster_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef cluster_py_hpp
#define cluster_py_hpp

#include "method_tags.hpp"

#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

void init_cluster(pybind11::class_<blocksci::Cluster> &cl);
void init_cluster_manager(pybind11::module &s);

struct AddClusterMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "index", +[](const Cluster &cluster) { return cluster.clusterNum; }, "The internal identifier of the cluster");
        func(method_tag, "tagged_addresses", &Cluster::taggedAddresses, "Given a dictionary of tags, return a range of TaggedAddress objects for any tagged addresses in the cluster", pybind11::arg("tagged_addresses"));
        func(method_tag, "size", &Cluster::getSize, "The number of addresses in the cluster");
        func(property_tag, "type_equiv_size", &Cluster::getTypeEquivSize, "The number of addresses in the cluster not counting type equivalent addresses");
        func(method_tag, "balance", &Cluster::calculateBalance, "Calculates the balance held by this cluster at the height (Defaults to the full chain)", pybind11::arg("height") = -1);
        func(method_tag, "out_txes_count", +[](Cluster &cluster) -> int64_t {
            return cluster.getOutputTransactions().size();
        }, "Return the number of transactions where this cluster was an output");
        func(method_tag, "in_txes_count", +[](Cluster &cluster) -> int64_t {
            return cluster.getInputTransactions().size();
        }, "Return the number of transactions where this cluster was an input");
        func(method_tag, "outs", &Cluster::getOutputs, "Returns a list of all outputs sent to this cluster");
        func(property_tag, "addresses", &Cluster::getAddresses, "Get a iterable over all the addresses in the cluster");
        func(method_tag, "count_of_type", &Cluster::countOfType, "Return the number of addresses of the given type in the cluster", pybind11::arg("address_type"));
    }
};

#endif /* cluster_py_hpp */
