//
//  blocksci.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <libcluster/cluster.hpp>
#include <libcluster/cluster_manager.hpp>

#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace blocksci;

uint64_t totalOutWithoutSelfChurn(const Block &block, ClusterManager &manager) {
    uint64_t total = 0;
    for (auto tx : block.txes()) {
        std::set<uint32_t> inputClusters;
        for (auto &input : tx.inputs()) {
            auto cluster = manager.getCluster(input.getAddress());
            if (cluster.getSize() < 30000) {
                inputClusters.insert(cluster.clusterNum);
            }
        }
        for (auto &output : tx.outputs()) {
            if ((!output.isSpent() || output.getSpendingTx()->blockHeight - block.height > 3) && inputClusters.find(manager.getCluster(output.getAddress()).clusterNum) == inputClusters.end()) {
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
     .def("cluster_with_address", &ClusterManager::getCluster)
     .def("cluster_count", &ClusterManager::clusterCount)
     .def("clusters", &ClusterManager::getClusters)
     .def("cluster_sizes", &ClusterManager::getClusterSizes)
     .def("tagged_clusters", &ClusterManager::taggedClusters)
     ;
     
     py::class_<Cluster>(m, "Cluster", "Class representing a cluster")
     .def_readonly("cluster_num", &Cluster::clusterNum)
     .def("__len__", &Cluster::getSize)
     .def("__eq__", &Cluster::operator==)
     .def("__hash__", [] (const Cluster &cluster) {
         return cluster.clusterNum;
     })
     .def_property_readonly("addresses", &Cluster::getAddresses)
     .def("tagged_addresses", &Cluster::taggedAddresses)
     .def("address_count", &Cluster::getAddressCount)
     ;
     
     py::class_<TaggedAddress>(m, "TaggedAddress")
     .def_readonly("address", &TaggedAddress::address)
     .def_readonly("tag", &TaggedAddress::tag)
     ;
     
     py::class_<TaggedCluster>(m, "TaggedCluster")
     .def_property_readonly("cluster", [](const TaggedCluster &tc) {
         return tc.cluster;
     })
     .def_readonly("tagged_addresses", &TaggedCluster::taggedAddresses)
     ;
     
     using cluster_range = boost::transformed_range<ClusterExpander, const boost::iterator_range<boost::iterators::counting_iterator<unsigned int>>>;
     py::class_<cluster_range>(m, "ClusterRange")
     .def("__len__", &cluster_range::size)
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
     
     using address_range = boost::iterator_range<const blocksci::Address *>;
     py::class_<address_range>(m, "AddressRange")
     .def("__len__", &address_range::size)
     /// Optional sequence protocol operations
     .def("__iter__", [](const address_range &range) { return py::make_iterator(range.begin(), range.end()); },
          py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
     .def("__getitem__", [](const address_range &range, int64_t i) {
         while (i < 0) {
             i = range.size() - i;
         }
         uint64_t posIndex = static_cast<uint64_t>(i);
         if (posIndex >= range.size())
             throw py::index_error();
         return range[i];
     })
     .def("__getitem__", [](const address_range &range, py::slice slice) -> py::list {
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
