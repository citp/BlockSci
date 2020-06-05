//
//  cluster_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "cluster_proxy_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/cluster/cluster.hpp>

struct AddClusterMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "index", +[](const Cluster &cluster) { return cluster.clusterNum; }, "The internal identifier of the cluster");
        func(method_tag, "tagged_addresses", &Cluster::taggedAddresses, "Given a dictionary of tags, return a range of TaggedAddress objects for any tagged addresses in the cluster", pybind11::arg("tagged_addresses"));
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

        func(method_tag, "outs", +[](Cluster &cluster) -> RawIterator<Output> {
            pybind11::print("Warning: `outs` is deprecated. Use `outputs` instead.");
            return cluster.getOutputs();
        }, "Returns an iterator over all outputs sent to this cluster");
        func(method_tag, "outputs", +[](Cluster &cluster) -> RawIterator<Output> {
            return cluster.getOutputs();
        }, "Returns an iterator over all outputs sent to this cluster");

        func(method_tag, "ins", +[](Cluster &cluster) -> RawIterator<Input> {
            pybind11::print("Warning: `ins` is deprecated. Use `inputs` instead.");
            return cluster.getInputs();
        }, "Returns an iterator over all inputs spent from this cluster");
        func(method_tag, "inputs", +[](Cluster &cluster) -> RawIterator<Input> {
            return cluster.getInputs();
        }, "Returns an iterator over all inputs spent from this cluster");

        func(property_tag, "addresses", +[](const Cluster &cluster) -> RawIterator<AnyScript> {
            return ranges::any_view<AnyScript>{ranges::views::transform(cluster.getAddresses(), [](Address && address) -> AnyScript {
                return address.getScript();
            })};
        }, "Get a iterable over all the addresses in the cluster");
        func(method_tag, "count_of_type", &Cluster::countOfType, "Return the number of addresses of the given type in the cluster", pybind11::arg("address_type"));
    }
};

void addClusterProxyMethods(AllProxyClasses<blocksci::Cluster> &cls) {
    cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);
    setupRangesProxy(cls);

    applyMethodsToProxy(cls.base, AddClusterMethods{});
    addProxyEqualityMethods(cls.base);
}
