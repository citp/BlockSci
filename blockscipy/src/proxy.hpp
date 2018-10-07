//
//  proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_hpp
#define proxy_hpp

#include "range_utils.hpp"

#include <any>
#include <functional>

template<typename T>
struct Proxy;

template<typename T>
Proxy<T> makeProxy() {
	return std::function<T(std::any &)>{[](std::any &t) -> T {
		return std::any_cast<T>(t);
	}};
}

template<typename T>
struct Proxy {
	using output_t = T;

	std::function<T(std::any &)> func;
	
	Proxy(const std::function<T(std::any &)> &func_) : func(func_) {}

	Proxy(const Proxy<T> & proxy) : func(proxy.func) {}

	Proxy(Proxy<T> && proxy) : func(std::move(proxy.func)) {}

	T operator()(std::any &t) const {
		return func(t);
	}

	T operator()(std::any && t) const {
		return func(t);
	}
};

template <typename F1, typename F2>
auto compose(F1 && f1, F2 && f2) {
	return [f1, f2](auto &&... args) {
		return f2(f1(std::forward<decltype(args)>(args)...));
	};
}

template <typename F1, typename F2, typename F3>
auto compose2(F1 && f1, F2 && f2, F3 && f3) {
	return [f1, f2, f3](auto &&... args) {
		return f3(f1(args...), f2(args...));
	};
}


#endif /* proxy_hpp */
