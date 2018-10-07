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

template <typename T>
Proxy<T> operator&&(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<T(std::any &)>{compose2(p1.func, p2.func, internal::_and{})};
}

template <typename T>
Proxy<T> operator||(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<T(std::any &)>{compose2(p1.func, p2.func, internal::_or{})};
}

struct AddProxyBooleanMethods {
	void operator()(pybind11::class_<Proxy<bool>> &cl) {
		using P = Proxy<bool>;
		cl
		.def("__and__", [](P &p, bool val) {
			return p && makeConstantProxy(val);
		})
		.def("__or__", [](P &p, bool val) {
			return p || makeConstantProxy(val);
		})
		.def("__and__", [](P &p1, P &p2) {
			return p1 && p2;
		})
		.def("__or__", [](P &p1, P &p2) {
			return p1 || p2;
		})
		.def("__invert__", [](P &p) -> Proxy<bool> {
			return std::function<bool(std::any &)>{[=](std::any &t) {
				return !p(t);
			}};
		})
		;
	}
};

#endif /* proxy_boolean_hpp */