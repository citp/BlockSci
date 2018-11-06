//
//  uint256_proxy.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "simple_proxies.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/range.hpp"
#include "proxy/basic.hpp"
#include "proxy/optional.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

namespace py = pybind11;

struct AddBytesMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
    	using namespace blocksci;
	    func(method_tag, "startswith", +[](const py::bytes &bytes, const std::string &str) -> bool {
			auto bytesString = static_cast<std::string>(bytes);
			return bytesString.find(str, 0) == 0;
		}, "Returns true if the byte string has the given prefix");
		func(method_tag, "endswith", +[](const py::bytes &bytes, const std::string &str) -> bool {
			auto bytesString = static_cast<std::string>(bytes);
		return bytesString.size() >= str.size() &&
			   bytesString.compare(bytesString.size() - str.size(), str.size(), str) == 0;
		}, "Returns true if the byte string has the given suffix");
    }
};

void addBytesProxyMethods(AllProxyClasses<py::bytes> &cls) {
	cls.applyToAll(AddProxyMethods{});
	addProxyOptionalMethods(cls.optional);
	applyMethodsToProxy(cls.base, AddBytesMethods{});
}