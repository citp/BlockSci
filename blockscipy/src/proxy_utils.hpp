//
//  proxy_utils.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_utils_hpp
#define proxy_utils_hpp

#include "proxy.hpp"

template <typename F1, typename F2>
auto lift(F1 &f1, F2 && f2) -> Proxy<decltype(f2(f1(std::declval<std::any &>())))> {
	return std::function<decltype(f2(f1(std::declval<std::any &>())))(std::any &)>{
		[f1, f2](std::any &v) {
			return f2(f1(v));
		}
	};
}

template <typename F1, typename F2>
auto liftGeneric(F1 &f1, F2 && f2) -> Proxy<decltype(f2(f1.getGeneric()(std::declval<std::any &>())))> {
	auto genericF1 = f1.getGeneric();
	return std::function<decltype(f2(genericF1(std::declval<std::any &>())))(std::any &)>{
		[f2, genericF1](std::any &v) {
			return f2(genericF1(v));
		}
	};
}

#endif /* proxy_utils_hpp */
