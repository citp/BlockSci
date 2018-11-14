//
//  proxy_utils.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_utils_hpp
#define proxy_utils_hpp

#include "proxy.hpp"

template <typename P, typename F>
auto lift(P && p, F && f) -> Proxy<decltype(f(p(std::declval<std::any &>())))> {
	return std::function<decltype(f(p(std::declval<std::any &>())))(std::any &)>{
		[p, f](std::any &v) {
			return f(p(v));
		}
	};
}

template <typename P, typename F>
auto liftGeneric(P && p, F && f) -> Proxy<decltype(f(p.getGeneric()(std::declval<std::any &>())))> {
	auto generic = p.getGeneric();
	return std::function<decltype(f(generic(std::declval<std::any &>())))(std::any &)>{
		[f, generic](std::any &v) {
			return f(generic(v));
		}
	};
}

#endif /* proxy_utils_hpp */
