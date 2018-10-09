//
//  boolean.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_boolean_hpp
#define proxy_boolean_hpp

#include "proxy.hpp"

void addProxyBooleanMethods(pybind11::class_<Proxy<bool>> &cl) {
	using P = Proxy<bool>;
	cl
	.def("__and__", [](P &p1, P &p2) -> P {
		return std::function<bool(std::any &)>{[p1, p2](std::any &v) -> bool {
			return p1(v) && p2(v);
		}};
	})
	.def("__or__", [](P &p1, P &p2) -> P {
		return std::function<bool(std::any &)>{[p1, p2](std::any &v) -> bool {
			return p1(v) || p2(v);
		}};
	})
	.def("__invert__", [](P &p) -> P {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return !p(t);
		}};
	})
	;
}

#endif /* proxy_boolean_hpp */