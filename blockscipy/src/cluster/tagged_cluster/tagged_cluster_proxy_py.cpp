//
//  tagged_cluster_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "tagged_cluster_proxy_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/cluster/cluster.hpp>

using namespace blocksci;

struct AddTaggedClusterMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "cluster", +[](const TaggedCluster &t) { return t.cluster; }, "Return the cluster object which has been tagged");
        func(property_tag, "tagged_addresses", +[](TaggedCluster &t) -> RawIterator<TaggedAddress> { return t.taggedAddresses; }, "Return the list of addresses inside the cluster which have been tagged");
    }
};

void addTaggedClusterProxyMethods(AllProxyClasses<blocksci::TaggedCluster> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddTaggedClusterMethods{});
    addProxyEqualityMethods(cls.base);
}
