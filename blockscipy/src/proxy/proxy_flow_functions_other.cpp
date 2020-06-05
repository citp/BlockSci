//
//  proxy_flow_functions_other.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/17/18.
//
//

#include "proxy_flow_functions_impl.hpp"

#include <chrono>

void addProxyFlowFunctionsOther(pybind11::module &m, pybind11::module &pm) {
	using namespace blocksci;

	addProxyFlowFunctions<AddressType::Enum>(m, pm);
	addProxyFlowFunctions<int64_t>(m, pm);
	addProxyFlowFunctions<bool>(m, pm);
	addProxyFlowFunctions<std::chrono::system_clock::time_point>(m, pm);
	addProxyFlowFunctions<uint256>(m, pm);
	addProxyFlowFunctions<uint160>(m, pm);
	addProxyFlowFunctions<pybind11::bytes>(m, pm);
	addProxyFlowFunctions<pybind11::list>(m, pm);
	addProxyFlowFunctions<std::string>(m, pm);

}
