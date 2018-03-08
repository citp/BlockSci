//
//  blocksci.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <libcluster/cluster.hpp>
#include <libcluster/cluster_manager.hpp>

#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace blocksci;

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
     .def(py::init<std::string>())
     .def("cluster_with_address", [](const ClusterManager &cm, const Address &address) {
        return cm.getCluster(address);
     }, "Return the cluster containing the given address")
     .def("cluster_with_script", [](const ClusterManager &cm, const Script &script) {
        return cm.getCluster(script);
     }, "Return the cluster containing the given script")
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
     .def_property_readonly("addresses", &Cluster::getDedupAddresses, "Get a iterable over all the dedup addresses in the cluster")
     .def("tagged_addresses", &Cluster::taggedDedupAddresses, "Given a dictionary of tags, return a list of TaggedDedupAddresses objects for any tagged dedup addresses in the cluster")
     .def("count_of_type", &Cluster::countOfType, "Return the number of dedup addresses of the given type in the cluster")
     ;
     
     py::class_<TaggedDedupAddress>(m, "TaggedDedupAddress")
     .def_readonly("address", &TaggedDedupAddress::address, "Return the dedup address object which has been tagged")
     .def_readonly("tag", &TaggedDedupAddress::tag, "Return the tag associated with the contained dedup address")
     ;
     
     py::class_<TaggedCluster>(m, "TaggedCluster")
     .def_property_readonly("cluster", [](const TaggedCluster &tc) {
         return tc.cluster;
     }, "Return the cluster object which has been tagged")
     .def_readonly("tagged_addresses", &TaggedCluster::taggedDedupAddresses, "Return the list of dedup addresses inside the cluster which have been tagged")
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
     
     using script_range = boost::iterator_range<const blocksci::DedupAddress *>;
     py::class_<script_range>(m, "DedupAddressRangeRange")
     .def("__len__", &script_range::size)
     /// Optional sequence protocol operations
     .def("__iter__", [](const script_range &range) { return py::make_iterator(range.begin(), range.end()); },
          py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
     .def("__getitem__", [](const script_range &range, int64_t i) {
         while (i < 0) {
             i = range.size() - i;
         }
         uint64_t posIndex = static_cast<uint64_t>(i);
         if (posIndex >= range.size())
             throw py::index_error();
         return range[i];
     })
     .def("__getitem__", [](const script_range &range, py::slice slice) -> py::list {
         size_t start, stop, step, slicelength;
         if (!slice.compute(range.size(), &start, &stop, &step, &slicelength))
             throw py::error_already_set();
         py::list txList;
         for (size_t i=0; i<slicelength; i+= step) {
             txList.append(range[start]);
             start += step;
         }
         return txList;
     })
     ;
 }
