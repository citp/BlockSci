//
//  python_proxies_main.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"
#include "proxy_py.hpp"

#include "chain/input/input_proxy_py.hpp"
#include "chain/output/output_proxy_py.hpp"
#include "chain/tx/tx_proxy_py.hpp"
#include "chain/block/block_proxy_py.hpp"
#include "scripts/equiv_address/equiv_address_proxy_py.hpp"

#include "cluster/cluster/cluster_proxy_py.hpp"
#include "cluster/tagged_cluster/tagged_cluster_proxy_py.hpp"
#include "cluster/tagged_address/tagged_address_proxy_py.hpp"

namespace py = pybind11;
using namespace blocksci;

MainProxies::MainProxies(py::module &m, py::class_<SimpleProxy> &proxySimpleCl, py::class_<OptionalProxy> &proxyOptionalCl, py::class_<IteratorProxy> &proxyIteratorCl, py::class_<RangeProxy> &proxyRangeCl) :
block(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
tx(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
input(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
output(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
equivAddress(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
cluster(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
taggedCluster(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl),
taggedAddress(m, proxySimpleCl, proxyOptionalCl, proxyIteratorCl, proxyRangeCl) {}


void setupMainProxies(MainProxies &proxies) {
    addBlockProxyMethods(proxies.block);
    addTxProxyMethods(proxies.tx);
    addInputProxyMethods(proxies.input);
    addOutputProxyMethods(proxies.output);
    addEquivAddressProxyMethods(proxies.equivAddress);
    
    addClusterProxyMethods(proxies.cluster);
    addTaggedClusterProxyMethods(proxies.taggedCluster);
    addTaggedAddressProxyMethods(proxies.taggedAddress);
}