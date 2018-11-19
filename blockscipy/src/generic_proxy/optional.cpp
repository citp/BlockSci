//
//  optional.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/31/18.
//
//

#include "optional.hpp"
#include "optional_map.hpp"
#include "proxy.hpp"
#include "proxy_utils.hpp"
#include "caster_py.hpp"

void addOptionalProxyMethods(pybind11::class_<OptionalProxy, GenericProxy> &cl, pybind11::module &) {
	addOptionalProxyMapMethods(cl);
	
	cl
	.def("has_value", [](OptionalProxy &p) -> Proxy<bool> {
		return liftGeneric(p, [](auto && val) {
			return std::forward<decltype(val)>(val).has_value();
		});
	})
	;
}

