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
#include "proxy_py_create.hpp"
#include "generic_sequence.hpp"

#include "chain/input/input_proxy_py.hpp"
#include "chain/output/output_proxy_py.hpp"
#include "chain/tx/tx_proxy_py.hpp"
#include "chain/block/block_proxy_py.hpp"
#include "scripts/equiv_address/equiv_address_proxy_py.hpp"

#include "cluster/cluster/cluster_proxy_py.hpp"
#include "cluster/tagged_cluster/tagged_cluster_proxy_py.hpp"
#include "cluster/tagged_address/tagged_address_proxy_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

namespace py = pybind11;
using namespace blocksci;

MainProxies::MainProxies(py::module &m) :
block(createProxyClasses<Block>(m)),
tx(createProxyClasses<Transaction>(m)),
input(createProxyClasses<Input>(m)),
output(createProxyClasses<Output>(m)),
equivAddress(createProxyClasses<EquivAddress>(m)),
cluster(createProxyClasses<Cluster>(m)),
taggedCluster(createProxyClasses<TaggedCluster>(m)),
taggedAddress(createProxyClasses<TaggedAddress>(m)) {}


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