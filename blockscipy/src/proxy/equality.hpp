//
//  equality.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_equality_hpp
#define proxy_equality_hpp

#include "proxy.hpp"

namespace internal {
	struct eq {
		template <typename A, typename B>
		bool operator()(A && a, B && b) const {
			return std::forward<A>(a) == std::forward<B>(b);
		}
	};

	struct neq {
		template <typename A, typename B>
		bool operator()(A && a, B && b) const {
			return std::forward<A>(a) != std::forward<B>(b);
		}
	};
}

template <typename T>
Proxy<bool> operator==(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<bool(std::any &)>{compose2(p1.func, p2.func, internal::eq{})};
}

template <typename T>
Proxy<bool> operator!=(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<bool(std::any &)>{compose2(p1.func, p2.func, internal::neq{})};
}

struct AddProxyEqualityMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T>> &cl) {
		using P = Proxy<T>;
		cl
		.def("__eq__", [](P &p, T &val) {
			return p == makeConstantProxy(val);
		})
		.def("__ne__", [](P &p, T &val) {
			return p != makeConstantProxy(val);
		})
		.def("__eq__", [](P &p1, P &p2) {
			return p1 == p2;
		})
		.def("__ne__", [](P &p1, P &p2) {
			return p1 != p2;
		})
		;
	}
};

#endif /* proxy_equality_hpp */