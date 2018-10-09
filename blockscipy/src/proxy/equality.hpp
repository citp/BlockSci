//
//  equality.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_equality_hpp
#define proxy_equality_hpp

#include "proxy.hpp"

template<typename T>
void addProxyEqualityMethods(pybind11::class_<Proxy<T>> &cl) {
	using P = Proxy<T>;
	cl
	.def("__eq__", [](P &p1, P &p2) -> Proxy<bool> {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return p1(t) == p2(t);
		}};
	})
	.def("__ne__", [](P &p1, P &p2) -> Proxy<bool> {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return p1(t) != p2(t);
		}};
	})
	;
}

#endif /* proxy_equality_hpp */