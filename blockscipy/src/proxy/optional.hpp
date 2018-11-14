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
void addProxyOptionalMethods(pybind11::class_<Proxy<ranges::optional<T>>, OptionalProxy> &cl) {
	using P = Proxy<ranges::optional<T>>;
	cl
	.def("or_value", [](P &p, Proxy<T> &vp) -> Proxy<T> {
		return lift(p, vp, [](auto && opt, auto && v) -> T {
			if (opt) {
				return *opt;
			} else {
				return v;
			}
		});
	})
	;
}

#endif /* proxy_optional_hpp */
