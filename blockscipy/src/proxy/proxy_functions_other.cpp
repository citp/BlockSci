//
//  proxy_functions_other.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/17/18.
//
//

#include "proxy_functions_impl.hpp"

#include <chrono>

void addProxyFunctionsOther(pybind11::module &m, pybind11::module &pm) {
	using namespace blocksci;

	addProxyFunctions<AddressType::Enum>(m, pm);
	addProxyFunctions<int64_t>(m, pm);
	addProxyFunctions<bool>(m, pm);
	addProxyFunctions<std::chrono::system_clock::time_point>(m, pm);
	addProxyFunctions<uint256>(m, pm);
	addProxyFunctions<uint160>(m, pm);
	addProxyFunctions<pybind11::bytes>(m, pm);
	addProxyFunctions<pybind11::list>(m, pm);
	addProxyFunctions<std::string>(m, pm);

}
