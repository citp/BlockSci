//
//  proxy_functions_core.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/17/18.
//
//

#include "proxy_functions_impl.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/chain/block.hpp>

void addProxyFunctionsCore(pybind11::module &m, pybind11::module &pm) {
	using namespace blocksci;

	addProxyFunctions<Block>(m, pm);
	addProxyFunctions<Transaction>(m, pm);
	addProxyFunctions<Input>(m, pm);
	addProxyFunctions<Output>(m, pm);
	addProxyFunctions<AnyScript>(m, pm);
	addProxyFunctions<EquivAddress>(m, pm);
	addProxyFunctions<Cluster>(m, pm);
	addProxyFunctions<TaggedCluster>(m, pm);
	addProxyFunctions<TaggedAddress>(m, pm);
}
