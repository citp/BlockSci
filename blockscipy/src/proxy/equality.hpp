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

template <typename T1, typename T2>
Proxy<T1, bool> operator==(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::eq{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator!=(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::neq{})};
}

struct AddProxyEqualityMethods {
	template<typename T, typename V>
	void operator()(pybind11::class_<Proxy<T, V>> &cl) {
		using P = Proxy<T, V>;
		cl
		.def("__eq__", [](P &p, V &val) {
			return p == makeConstantProxy<T>(val);
		})
		.def("__ne__", [](P &p, V &val) {
			return p != makeConstantProxy<T>(val);
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