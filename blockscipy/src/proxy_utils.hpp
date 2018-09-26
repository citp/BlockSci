//
//  proxy_utils.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_utils_hpp
#define proxy_utils_hpp

#include "proxy.hpp"
#include "proxy_operators.hpp"
#include <functional>

template<typename T>
Proxy<T, T> makeProxy() {
	return std::function<T(T &)>{[](T &t) -> T {
		return t;
	}};
}

template <typename T1, typename T2, typename F>
auto lift(const Proxy<T1, T2> &p, F && f) -> Proxy<T1, decltype(f(std::declval<T2>()))> {
	return std::function<decltype(f(std::declval<T2>()))(T1 &)>{compose(p.func, f)};
}

#endif /* proxy_utils_hpp */
