//
//  boolean.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_boolean_hpp
#define proxy_boolean_hpp

#include "proxy.hpp"

namespace internal {
	struct _and {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(a && b) {
			return std::forward<A>(a) && std::forward<B>(b);
		}
	};

	struct _or {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(a || b) {
			return std::forward<A>(a) || std::forward<B>(b);
		}
	};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator&&(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::_and{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator||(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::_or{})};
}

struct AddProxyBooleanMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T, bool>> &cl) {
		using P = Proxy<T, bool>;
		cl
		.def("__and__", [](P &p, bool val) {
			return p && makeConstantProxy<T>(val);
		})
		.def("__or__", [](P &p, bool val) {
			return p || makeConstantProxy<T>(val);
		})
		.def("__and__", [](P &p1, P &p2) {
			return p1 && p2;
		})
		.def("__or__", [](P &p1, P &p2) {
			return p1 || p2;
		})
		.def("__invert__", [](P &p) -> Proxy<T, bool> {
			return std::function<bool(T &)>{[=](T &t) {
				return !p(t);
			}};
		})
		;
	}
};

#endif /* proxy_boolean_hpp */