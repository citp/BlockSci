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

#include <pybind11/iostream.h>

namespace py = pybind11;

int64_t totalOutWithoutSelfChurn(const blocksci::Block &block, blocksci::ClusterManager &manager) {
    int64_t total = 0;
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

template <typename T>
auto addClusterRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addClusterMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each cluster: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

template <typename T>
auto addTaggedAddressRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addTaggedAddressMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each tagged address: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
}

template <typename T>
auto addTaggedClusterRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addTaggedClusterMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each tagged cluster: " << docstring;
        return strdup(ss.str().c_str());
    });
    return cl;
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
    .def("cluster_with_address", [](const ClusterManager &cm, const Address &address) -> Cluster {
       return cm.getCluster(address);
    }, "Return the cluster containing the given address")
    .def("clusters", [](ClusterManager &cm) -> ranges::any_view<Cluster, ranges::category::random_access> {
        return cm.getClusters();
    }, "Get a list of all clusters (The list is lazy so there is no cost to calling this method)")
    .def("tagged_clusters", [](ClusterManager &cm, const std::unordered_map<blocksci::Address, std::string> &tags) -> ranges::any_view<TaggedCluster> {
        return cm.taggedClusters(tags);
    }, "Given a dictionary of tags, return a list of TaggedCluster objects for any clusters containing tagged scripts")
    ;
    
    py::class_<Cluster> clusterCl(s, "Cluster", "Class representing a cluster");
    clusterCl
    .def_readonly("cluster_num", &Cluster::clusterNum)
    .def("__len__", &Cluster::getSize)
    .def("__eq__", &Cluster::operator==)
    .def("__hash__", [] (const Cluster &cluster) {
        return cluster.clusterNum;
    })
    .def("ins", &Cluster::getInputs, "Returns a list of all inputs spent from this cluster")
    .def("txes", &Cluster::getTransactions, "Returns a list of all transactions involving this cluster")
    .def("in_txes",&Cluster::getInputTransactions, "Returns a list of all transaction where this cluster was an input")
    .def("out_txes", &Cluster::getOutputTransactions, "Returns a list of all transaction where this cluster was an output")
    ;
    
    addClusterMethods(clusterCl, [](auto func) {
        return applyMethodsToSelf<Cluster>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<TaggedAddress> taggedAddressCl(s, "TaggedAddress");
    addTaggedAddressMethods(taggedAddressCl, [](auto func) {
        return applyMethodsToSelf<Cluster>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    py::class_<TaggedCluster> taggedClusterCl(s, "TaggedCluster");
    addTaggedClusterMethods(taggedClusterCl, [](auto func) {
        return applyMethodsToSelf<Cluster>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    

    addClusterRange<ranges::any_view<Cluster, ranges::category::random_access>>(s, "ClusterRange");
    addClusterRange<ranges::any_view<Cluster>>(s, "AnyClusterRange");
    addTaggedAddressRange<ranges::any_view<TaggedAddress, ranges::category::random_access>>(s, "TaggedAddressRange");
    addTaggedAddressRange<ranges::any_view<TaggedAddress>>(s, "AnyTaggedAddressRange");
    addTaggedClusterRange<ranges::any_view<TaggedCluster, ranges::category::random_access>>(s, "TaggedClusterRange");
    addTaggedClusterRange<ranges::any_view<TaggedCluster>>(s, "AnyTaggedClusterRange");
}
