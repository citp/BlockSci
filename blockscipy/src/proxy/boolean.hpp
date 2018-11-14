//
//  boolean.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_boolean_hpp
#define proxy_boolean_hpp

#include "proxy.hpp"
#include "proxy_type_check.hpp"

template<typename Class>
void addProxyBooleanMethods(Class &cl) {
	using P = typename Class::type;
	using T = typename P::output_t;
	cl
	.def("__and__", [](P &p1, P &p2) -> P {
		// Use this instead of lift to take advantage of short-circuit
		proxyTypeCheck(p1.getSourceType(), p2.getSourceType());
		return {std::function<bool(std::any &)>{[p1, p2](std::any &v) -> bool {
			return p1(v) && p2(v);
		}}, p1.getSourceType()};
	})
	.def("__or__", [](P &p1, P &p2) -> P {
		// Use this instead of lift to take advantage of short-circuit
		proxyTypeCheck(p1.getSourceType(), p2.getSourceType());
		return {std::function<bool(std::any &)>{[p1, p2](std::any &v) -> bool {
			return p1(v) || p2(v);
		}}, p1.getSourceType()};
	})
	.def("__invert__", [](P &p) -> P {
		return lift(p, [](auto && v) -> T {
			return !std::forward<decltype(v)>(v);
		});
	})
	;
}

#endif /* proxy_boolean_hpp */