//
//  proxy_utils.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_utils_hpp
#define proxy_utils_hpp

#include "proxy.hpp"

template<typename V>
Proxy<V> makeConstantProxy(const V &val) {
	return std::function<V(std::any &)>{[=](std::any &) -> V {
		return val;
	}};
}

template <typename T, typename F>
auto lift(const Proxy<T> &p, F && f) -> Proxy<decltype(f(std::declval<T>()))> {
	return std::function<decltype(f(std::declval<T>()))(std::any &)>{compose(p.func, std::forward<F>(f))};
}

#endif /* proxy_utils_hpp */
