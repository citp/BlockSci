//
//  blocksci.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "cluster_py.hpp"
#include "variant_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/cluster/cluster_manager.hpp>

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <pybind11/stl.h>
#include <pybind11/iostream.h>

namespace py = pybind11;

uint64_t totalOutWithoutSelfChurn(const blocksci::Block &block, blocksci::ClusterManager &manager) {
    uint64_t total = 0;
    RANGES_FOR(auto tx, block) {
        std::set<uint32_t> inputClusters;
        RANGES_FOR(auto input, tx.inputs()) {
            auto cluster = manager.getCluster(input.getAddress());
            if (cluster.getSize() < 30000) {
                inputClusters.insert(cluster.clusterNum);
            }
        }
        RANGES_FOR(auto output, tx.outputs()) {
            if ((!output.isSpent() || output.getSpendingTx()->blockHeight - block.height() > 3) && inputClusters.find(manager.getCluster(output.getAddress()).clusterNum) == inputClusters.end()) {
                total += output.getValue();
            }
        }
    }
    return total;
}



void init_cluster(pybind11::module &m) {
    using namespace blocksci;

    auto s = m.def_submodule("cluster");
    s
    .def("total_without_self_churn", totalOutWithoutSelfChurn)
    .def("create_clustering", [](Blockchain &chain, const heuristics::ChangeHeuristic &heuristic, const std::string &location, bool shouldOverwrite) {
        py::scoped_ostream_redirect stream(
            std::cout,
            py::module::import("sys").attr("stdout")
        );
        return ClusterManager::createClustering(chain, heuristic, location, shouldOverwrite);
    }, py::arg("chain"), py::arg("heuristic"), py::arg("location"), py::arg("should_overwrite") = false)
    ;

    
    py::class_<ClusterManager>(s, "ClusterManager", "Class managing the cluster dat")
    .def(py::init([](std::string arg, blocksci::Blockchain &chain) {
       return ClusterManager(arg, chain.getAccess());
    }))
    .def("cluster_with_address", [](const ClusterManager &cm, const Address &address) {
       return cm.getCluster(address);
    }, "Return the cluster containing the given address")
    .def("cluster_count", &ClusterManager::clusterCount, "Get the total number of clusters")
    .def("clusters", &ClusterManager::getClusters
       , "Get a list of all clusters (The list is lazy so there is no cost to calling this method)")
    .def("cluster_sizes", &ClusterManager::getClusterSizes, "Get a list of all cluster sizes (This is quite slow)")
    .def("tagged_clusters", &ClusterManager::taggedClusters
       , "Given a dictionary of tags, return a list of TaggedCluster objects for any clusters containing tagged scripts")
    ;
    
    py::class_<Cluster>(s, "Cluster", "Class representing a cluster")
    .def_readonly("cluster_num", &Cluster::clusterNum)
    .def("__len__", &Cluster::getSize)
    .def("__eq__", &Cluster::operator==)
    .def("__hash__", [] (const Cluster &cluster) {
        return cluster.clusterNum;
    })
    .def_property_readonly("addresses", [](const Cluster &cluster) {
        py::list ret;
        for (const auto &address : cluster.getAddresses()) {
            ret.append(address.getScript().wrapped);
        }
        return ret;
    }, "Get a iterable over all the addresses in the cluster")
    .def("tagged_addresses", &Cluster::taggedAddresses, "Given a dictionary of tags, return a list of TaggedAddress objects for any tagged addresses in the cluster")
    .def("count_of_type", &Cluster::countOfType, "Return the number of addresses of the given type in the cluster")
    .def("balance", &Cluster::calculateBalance, py::arg("height") = -1, "Calculates the balance held by this cluster at the height (Defaults to the full chain)")
    .def("outs", &Cluster::getOutputs, "Returns a list of all outputs sent to this cluster")
    .def("ins", &Cluster::getInputs, "Returns a list of all inputs spent from this cluster")
    .def("txes", &Cluster::getTransactions, "Returns a list of all transactions involving this cluster")
    .def("in_txes",&Cluster::getInputTransactions, "Returns a list of all transaction where this cluster was an input")
    .def("out_txes", &Cluster::getOutputTransactions, "Returns a list of all transaction where this cluster was an output")
    .def("out_txes_count", [](const Cluster &address) {
        return address.getOutputTransactions().size();
    }, "Return the number of transactions where this cluster was an output")
    .def("in_txes_count", [](const Cluster &address) {
        return address.getInputTransactions().size();
    }, "Return the number of transactions where this cluster was an input")
    ;
    ;
    
    py::class_<TaggedAddress>(s, "TaggedAddress")
    .def_property_readonly("address", [](const TaggedAddress &tagged) {
        return tagged.address.getScript().wrapped;
    }, "Return the address object which has been tagged")
    .def_readonly("tag", &TaggedAddress::tag, "Return the tag associated with the contained address")
    ;
    
    py::class_<TaggedCluster>(s, "TaggedCluster")
    .def_property_readonly("cluster", [](const TaggedCluster &tc) {
        return tc.cluster;
    }, "Return the cluster object which has been tagged")
    .def_readonly("tagged_addresses", &TaggedCluster::taggedAddresses, "Return the list of addresses inside the cluster which have been tagged")
    ;
    

    addRangeClass<cluster_range>(s, "ClusterRange");
}
