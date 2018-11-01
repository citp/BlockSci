//
//  optional.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/31/18.
//
//

#include "optional.hpp"
#include "optional_map.hpp"

void addOptionalProxyMethods(pybind11::class_<OptionalProxy> &cl) {
	addOptionalProxyMapMethods(cl);
	
	cl
	.def("has_value", [](OptionalProxy &p) -> Proxy<bool> {
		auto generic = p.getGeneric();
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return generic(t).has_value();
		}};
	})
	;
}

