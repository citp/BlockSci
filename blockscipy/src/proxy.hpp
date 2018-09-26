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


#endif /* proxy_hpp */
