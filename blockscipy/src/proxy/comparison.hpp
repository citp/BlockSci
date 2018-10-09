//
//  comparison.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_comparison_hpp
#define proxy_comparison_hpp

#include "proxy.hpp"

template<typename T>
void addProxyComparisonMethods(pybind11::class_<Proxy<T>> &cl) {
	using P = Proxy<T>;
	cl
	.def("__lt__", [](P &p1, P &p2) -> Proxy<bool> {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return p1(t) < p2(t);
		}};
	})
	.def("__le__", [](P &p1, P &p2) -> Proxy<bool> {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return p1(t) <= p2(t);
		}};
	})
	.def("__gt__", [](P &p1, P &p2) -> Proxy<bool> {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return p1(t) > p2(t);
		}};
	})
	.def("__ge__", [](P &p1, P &p2) -> Proxy<bool> {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return p1(t) >= p2(t);
		}};
	})
	;
}

#endif /* proxy_comparison_hpp */