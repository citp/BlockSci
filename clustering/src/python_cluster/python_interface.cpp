//
//  blocksci.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <libcluster/cluster.hpp>
#include <libcluster/cluster_manager.hpp>

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace blocksci;

#include <mpark/variant.hpp>

namespace pybind11 { namespace detail {
    template <typename... Ts>
    struct type_caster<mpark::variant<Ts...>> : variant_caster<mpark::variant<Ts...>> {};
    
    // Specifies the function used to visit the variant -- `apply_visitor` instead of `visit`
    template <>
    struct visit_helper<mpark::variant> {
        template <typename... Args>
        static auto call(Args &&...args) -> decltype(mpark::visit(args...)) {
            return mpark::visit(args...);
        }
    };
}}

uint64_t totalOutWithoutSelfChurn(const Block &block, ClusterManager &manager) {
    uint64_t total = 0;
    for (auto tx : block) {
        std::set<uint32_t> inputClusters;
        for (auto input : tx.inputs()) {
            auto cluster = manager.getCluster(input.getAddress());
            if (cluster.getSize() < 30000) {
                inputClusters.insert(cluster.clusterNum);
            }
        }
        for (auto output : tx.outputs()) {
            if ((!output.isSpent() || output.getSpendingTx()->blockHeight - block.height() > 3) && inputClusters.find(manager.getCluster(output.getAddress()).clusterNum) == inputClusters.end()) {
                total += output.getValue();
            }
        }
    }
    return total;
}


 PYBIND11_MODULE(cluster_python, m) {
    m.def("total_without_self_churn", totalOutWithoutSelfChurn);
    
    py::class_<ClusterManager>(m, "ClusterManager", "Class managing the cluster dat")
    .def(py::init([](std::string arg, const blocksci::Blockchain &chain) {
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
    
    py::class_<Cluster>(m, "Cluster", "Class representing a cluster")
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
    
    py::class_<TaggedAddress>(m, "TaggedAddress")
    .def_property_readonly("address", [](const TaggedAddress &tagged) {
        return tagged.address.getScript().wrapped;
    }, "Return the address object which has been tagged")
    .def_readonly("tag", &TaggedAddress::tag, "Return the tag associated with the contained address")
    ;
    
    py::class_<TaggedCluster>(m, "TaggedCluster")
    .def_property_readonly("cluster", [](const TaggedCluster &tc) {
        return tc.cluster;
    }, "Return the cluster object which has been tagged")
    .def_readonly("tagged_addresses", &TaggedCluster::taggedAddresses, "Return the list of addresses inside the cluster which have been tagged")
    ;
    
    py::class_<cluster_range>(m, "ClusterRange")
    .def("__len__", [](const cluster_range &range) {
       return range.size();
    })
    /// Optional sequence protocol operations
    .def("__iter__", [](const cluster_range &range) { return py::make_iterator(range.begin(), range.end()); },
         py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
    .def("__getitem__", [](const cluster_range &range, int64_t i) {
        while (i < 0) {
            i = range.size() - i;
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= range.size())
            throw py::index_error();
        return range[i];
    })
    .def("__getitem__", [](const cluster_range &range, py::slice slice) -> py::list {
        size_t start, stop, step, slicelength;
        if (!slice.compute(range.size(), &start, &stop, &step, &slicelength))
            throw py::error_already_set();
        py::list txList;
        for (size_t i=0; i<slicelength; ++i) {
            txList.append(range[start]);
            start += step;
        }
        return txList;
    })
    ;
 }
