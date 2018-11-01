//
//  optional_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 10/31/18.
//


#ifndef proxy_optional_map_hpp
#define proxy_optional_map_hpp

#include "proxy.hpp"

#include <pybind11/pybind11.h>

template<typename R>
Proxy<ranges::optional<R>> mapOptional(OptionalProxy &seq, Proxy<R> &p2) {
	auto generic = seq.getGeneric();
	return std::function<ranges::optional<R>(std::any &)>{[=](std::any &val) -> ranges::optional<R> {
		auto v = generic(val);
		if (v) {
			return p2(*v);
		} else {
			return ranges::nullopt;
		}
	}};
}

void addOptionalProxyMapMethods(pybind11::class_<OptionalProxy> &cl);

#endif /* proxy_optional_map_hpp */
