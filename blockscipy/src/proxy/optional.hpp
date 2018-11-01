//
//  optional.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_optional_hpp
#define proxy_optional_hpp

#include "proxy.hpp"

#include <pybind11/pybind11.h>

template<typename T>
void addProxyOptionalMethods(pybind11::class_<Proxy<ranges::optional<T>>> &cl) {
	using P = Proxy<ranges::optional<T>>;
	cl
	.def("or_value", [](P &p, Proxy<T> &v) -> Proxy<T> {
		return std::function<T(std::any &)>{[=](std::any &t) {
			auto opt = p(t);
			if (opt) {
				return *opt;
			} else {
				return v(t);
			}
		}};
	})
	;
}

void addOptionalProxyMethods(pybind11::class_<OptionalProxy> &cl);

#endif /* proxy_optional_hpp */
