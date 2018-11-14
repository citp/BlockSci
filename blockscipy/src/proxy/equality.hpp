//
//  equality.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_equality_hpp
#define proxy_equality_hpp

#include "proxy.hpp"

template<typename Class>
void addProxyEqualityMethods(Class &cl) {
	using P = typename Class::type;
	cl
	.def("__eq__", [](P &p1, P &p2) -> Proxy<bool> {
		return lift(p1, p2, [](auto && v1, auto && v2) -> bool {
			return std::forward<decltype(v1)>(v1) == std::forward<decltype(v2)>(v2);
		});
	})
	.def("__ne__", [](P &p1, P &p2) -> Proxy<bool> {
		return lift(p1, p2, [](auto && v1, auto && v2) -> bool {
			return std::forward<decltype(v1)>(v1) != std::forward<decltype(v2)>(v2);
		});
	})
	;
}

#endif /* proxy_equality_hpp */