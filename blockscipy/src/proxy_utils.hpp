//
//  proxy_utils.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_utils_hpp
#define proxy_utils_hpp

#include "proxy.hpp"

template <typename P1, typename P2, typename F>
auto lift(P1 && p1, P2 && p2, F && f) -> Proxy<decltype(f(p1(std::declval<std::any &>()), p2(std::declval<std::any &>())))> {
	return std::function<decltype(f(p1(std::declval<std::any &>()), p2(std::declval<std::any &>())))(std::any &)>{
		[p1, p2, f](std::any &v) {
			return f(p1(v), p2(v));
		}
	};
}

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

template <typename P, typename F>
auto liftSequence(P && p, F && f) -> Proxy<decltype(f(p.getIteratorFunc()(std::declval<std::any &>())))> {
	auto generic = p.getIteratorFunc();
	return std::function<decltype(f(generic(std::declval<std::any &>())))(std::any &)>{
		[f, generic](std::any &v) {
			return f(generic(v));
		}
	};
}

template <typename T>
Proxy<T> compose(Proxy<T> &p, GenericProxy &g) {
	return std::function<T(std::any &)>{[generic = g.getGenericAny(), p](std::any &v) -> T {
		return p(generic(v));
	}};
}

#endif /* proxy_utils_hpp */
