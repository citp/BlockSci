//
//  cluster_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "cluster_proxy_py.hpp"
#include "cluster_properties_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

struct AddClusterProxyMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;

        func(method_tag, "tagged_addresses", &Cluster::taggedAddresses, "Given a dictionary of tags, return a range of TaggedAddress objects for any tagged addresses in the cluster", pybind11::arg("tagged_addresses"));
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
    }
};

void addClusterProxyMethods(AllProxyClasses<blocksci::Cluster> &cls) {
    cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);
    setupRangesProxy(cls);

    applyMethodsToProxy(cls.base, AddClusterMethods{});
    applyMethodsToProxy(cls.base, AddClusterProxyMethods{});
    addProxyEqualityMethods(cls.base);
}
