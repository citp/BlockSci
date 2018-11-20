//
//  proxy_flow_functions_core.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/20/18.
//
//

#include "proxy_flow_functions_impl.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/chain/block.hpp>

void addProxyFlowFunctionsCore(pybind11::module &m, pybind11::module &pm) {
	using namespace blocksci;

	addProxyFlowFunctions<Block>(m, pm);
	addProxyFlowFunctions<Transaction>(m, pm);
	addProxyFlowFunctions<Input>(m, pm);
	addProxyFlowFunctions<Output>(m, pm);
	addProxyFlowFunctions<AnyScript>(m, pm);
	addProxyFlowFunctions<EquivAddress>(m, pm);
	addProxyFlowFunctions<Cluster>(m, pm);
	addProxyFlowFunctions<TaggedCluster>(m, pm);
	addProxyFlowFunctions<TaggedAddress>(m, pm);
}
