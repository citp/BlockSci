//
//  proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_hpp
#define proxy_hpp

#include <functional>

template<typename T1, typename T2>
struct Proxy;

template<typename T>
Proxy<T, T> makeProxy() {
	return std::function<T(T &)>{[](T &t) -> T {
		return t;
	}};
}

template<typename T1, typename T2>
struct Proxy {
	using output_t = T2;
	using input_t = T1;

	std::function<T2(T1 &)> func;

	
	Proxy(const std::function<T2(T1 &)> &func_) : func(func_) {}

	Proxy(const Proxy<T1, T2> & proxy) : func(proxy.func) {}

	Proxy(Proxy<T1, T2> && proxy) : func(std::move(proxy.func)) {}

	T2 operator()(T1 &t) const {
		return func(t);
	}

	T2 operator()(T1 && t) const {
		return func(t);
	}
};

template <typename Func, typename T>
auto curry(Func func, T && t) {
	return std::bind(func, std::placeholders::_1, std::forward<T>(t));
}

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
