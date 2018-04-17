//
//  cluster_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef cluster_py_hpp
#define cluster_py_hpp

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>

void init_cluster(pybind11::module &m);

template <typename Class, typename FuncApplication, typename FuncDoc>
void addClusterMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("cluster_num", func([](const Cluster &cluster) -> int64_t {
        return cluster.clusterNum;
    }), func2("The internal identifier of the cluster"))
    .def("tagged_addresses", func([](const Cluster &cluster, const std::unordered_map<blocksci::Address, std::string> &tags) -> ranges::any_view<TaggedAddress> {
        return cluster.taggedAddresses(tags);
    }), func2("Given a dictionary of tags, return a range of TaggedAddress objects for any tagged addresses in the cluster"))
    .def("size", func([](Cluster &cluster) -> int64_t {
        return cluster.getSize();
    }), func2("The number of addresses in the cluster"))
    .def("balance", func([](Cluster &cluster, int height) -> int64_t {
        return cluster.calculateBalance(height);
    }), pybind11::arg("height") = -1, func2("Calculates the balance held by this cluster at the height (Defaults to the full chain)"))
    .def("out_txes_count", func([](Cluster &cluster) -> int64_t {
        return cluster.getOutputTransactions().size();
    }), func2("Return the number of transactions where this cluster was an output"))
    .def("in_txes_count", func([](Cluster &cluster) -> int64_t {
        return cluster.getInputTransactions().size();
    }), func2("Return the number of transactions where this cluster was an input"))
    .def("outs", func([](Cluster &cluster) -> ranges::any_view<Output> {
        return cluster.getOutputs();
    }), func2("Returns a list of all outputs sent to this cluster"))
    .def_property_readonly("addresses", func([](Cluster &cluster) -> ranges::any_view<AnyScript> {
        return cluster.getAddresses() | ranges::view::transform([](Address && address) {
            return address.getScript();
        });
    }), func2("Get a iterable over all the addresses in the cluster"))
    .def("count_of_type", func([](Cluster &cluster, blocksci::AddressType::Enum type) -> int64_t {
        return cluster.countOfType(type);
    }), func2("Return the number of addresses of the given type in the cluster"))
    ;
}

template <typename Class, typename FuncApplication, typename FuncDoc>
void addTaggedAddressMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("address", func([](const TaggedAddress &tagged) -> AnyScript {
        return tagged.address.getScript();
    }), func2("Return the address object which has been tagged"))
    .def_readonly("tag", func([](const TaggedAddress &tagged) -> std::string {
        return tagged.tag;
    }), func2("Return the tag associated with the contained address"))
    ;
}

template <typename Class, typename FuncApplication, typename FuncDoc>
void addTaggedClusterMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("cluster", func([](TaggedCluster &tc) -> Cluster {
        return tc.cluster;
    }), func("Return the cluster object which has been tagged"))
    .def_readonly("tagged_addresses",func([](TaggedCluster &tc, const std::unordered_map<blocksci::Address, std::string> &tags) -> ranges::any_view<TaggedAddress> {
        return tc.taggedAddresses(tags);
    }), func2("Return the list of addresses inside the cluster which have been tagged"))
    ;
}

#endif /* cluster_py_hpp */
