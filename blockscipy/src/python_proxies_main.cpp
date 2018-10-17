//
//  python_proxies_main.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
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

void setupMainProxies(py::module &m, py::class_<ProxySequence<ranges::category::input>> &proxyIteratorCl, py::class_<ProxySequence<random_access_sized>> &proxyRangeCl) {

    AllProxyClasses<Block> blockProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<Transaction> txProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<Input> inputProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<Output> outputProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<EquivAddress> equivAddressProxyCls(m, proxyIteratorCl, proxyRangeCl);

    AllProxyClasses<Cluster> clusterProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<TaggedCluster> taggedClusterProxyCls(m, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<TaggedAddress> taggedAddressProxyCls(m, proxyIteratorCl, proxyRangeCl);

    addBlockProxyMethods(blockProxyCls);
    addTxProxyMethods(txProxyCls);
    addInputProxyMethods(inputProxyCls);
    addOutputProxyMethods(outputProxyCls);
    addEquivAddressProxyMethods(equivAddressProxyCls);
    
    addClusterProxyMethods(clusterProxyCls);
    addTaggedClusterProxyMethods(taggedClusterProxyCls);
    addTaggedAddressProxyMethods(taggedAddressProxyCls);
}