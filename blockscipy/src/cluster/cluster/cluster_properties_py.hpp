//
//  cluster_properties_py.hpp
//  blockscipy
//
//  Created by Malte Moeser on 8/26/19.
//

#ifndef cluster_properties_py_h
#define cluster_properties_py_h

#include "method_tags.hpp"

#include <blocksci/cluster/cluster.hpp>

#include <pybind11/operators.h>


struct AddClusterMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;

        func(property_tag, "index", +[](const Cluster &cluster) { return cluster.clusterNum; }, "The internal identifier of the cluster");
        func(method_tag, "address_count", &Cluster::getSize, "The number of addresses in the cluster");
        func(property_tag, "type_equiv_size", &Cluster::getTypeEquivSize, "The number of addresses in the cluster not counting type equivalent addresses");
        func(method_tag, "balance", &Cluster::calculateBalance, "Calculates the balance held by this cluster at the height (Defaults to the full chain)", pybind11::arg("height") = -1);

        func(method_tag, "out_txes_count", +[](Cluster &cluster) -> int64_t {
            pybind11::print("Warning: `out_txes_count` is deprecated. Use `output_txes_count` instead.");
            return cluster.getOutputTransactions().size();
        }, "Return the number of transactions where an address in this cluster was used in an output");
        func(method_tag, "output_txes_count", +[](Cluster &cluster) -> int64_t {
            return cluster.getOutputTransactions().size();
        }, "Return the number of transactions where an address in this cluster was used in an output");

        func(method_tag, "in_txes_count", +[](Cluster &cluster) -> int64_t {
            pybind11::print("Warning: `in_txes_count` is deprecated. Use `input_txes_count` instead.");
            return cluster.getInputTransactions().size();
        }, "Return the number of transactions where this cluster was an input");
        func(method_tag, "input_txes_count", +[](Cluster &cluster) -> int64_t {
            return cluster.getInputTransactions().size();
        }, "Return the number of transactions where this cluster was an input");

        func(method_tag, "count_of_type", &Cluster::countOfType, "Return the number of addresses of the given type in the cluster", pybind11::arg("address_type"));
    }
};

#endif /* cluster_properties_py_h */
