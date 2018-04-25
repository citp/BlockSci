//
//  cluster_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef cluster_py_hpp
#define cluster_py_hpp

#include "ranges_py.hpp"

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>

void init_cluster(pybind11::module &m);

struct AddClusterMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddClusterMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        using namespace blocksci;
        cl
        .def_property_readonly("index", func([](const Cluster &cluster) { return cluster.clusterNum; }), func2("The internal identifier of the cluster"))
        .def("tagged_addresses", func(&Cluster::taggedAddresses), func2("Given a dictionary of tags, return a range of TaggedAddress objects for any tagged addresses in the cluster"))
        .def("size", func(&Cluster::getSize), func2("The number of addresses in the cluster"))
        .def_property_readonly("type_equiv_size", func(&Cluster::getTypeEquivSize), func2("The number of addresses in the cluster not counting type equivalent addresses"))
        .def("balance", func(&Cluster::calculateBalance), pybind11::arg("height") = -1, func2("Calculates the balance held by this cluster at the height (Defaults to the full chain)"))
        .def("out_txes_count", func([](Cluster &cluster) -> int64_t {
            return cluster.getOutputTransactions().size();
        }), func2("Return the number of transactions where this cluster was an output"))
        .def("in_txes_count", func([](Cluster &cluster) -> int64_t {
            return cluster.getInputTransactions().size();
        }), func2("Return the number of transactions where this cluster was an input"))
        .def("outs", func(&Cluster::getOutputs), func2("Returns a list of all outputs sent to this cluster"))
        .def_property_readonly("addresses", func([](Cluster &cluster) -> ranges::any_view<AnyScript> {
            return cluster.getAddresses() | ranges::view::transform([](Address && address) {
                return address.getScript();
            });
        }), func2("Get a iterable over all the addresses in the cluster"))
        .def("count_of_type", func(&Cluster::countOfType), func2("Return the number of addresses of the given type in the cluster"))
        ;
    }
};

struct AddTaggedAddressMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddTaggedAddressMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        using namespace blocksci;
        cl
        .def_property_readonly("address", func([](const TaggedAddress &t) { return t.address; }), func2("Return the address object which has been tagged"))
        .def_property_readonly("tag", func([](const TaggedAddress &t) { return t.tag; }), func2("Return the tag associated with the contained address"))
        ;
    }
};

struct AddTaggedClusterMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddTaggedClusterMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        using namespace blocksci;
        cl
        .def_property_readonly("cluster", func([](const TaggedCluster &t) { return t.cluster; }), func2("Return the cluster object which has been tagged"))
        .def_property_readonly("tagged_addresses", func([](const TaggedCluster &t) { return t.taggedAddresses; }), func2("Return the list of addresses inside the cluster which have been tagged"))
        ;
    }
};

#endif /* cluster_py_hpp */
