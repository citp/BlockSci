//
//  comparison.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_comparison_hpp
#define proxy_comparison_hpp

#include "proxy.hpp"

namespace internal {
	struct gt {
		template <typename A, typename B>
		bool operator()(A && a, B && b) const {
			return std::forward<A>(a) > std::forward<B>(b);
		}
	};

	struct gte {
		template <typename A, typename B>
		bool operator()(A && a, B && b) const {
			return std::forward<A>(a) >= std::forward<B>(b);
		}
	};

	struct lt {
		template <typename A, typename B>
		bool operator()(A && a, B && b) const {
			return std::forward<A>(a) < std::forward<B>(b);
		}
	};

	struct lte {
		template <typename A, typename B>
		bool operator()(A && a, B && b) const {
			return std::forward<A>(a) <= std::forward<B>(b);
		}
	};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator>(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::gt{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator>=(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::gte{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator<(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::lt{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator<=(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::lte{})};
}

struct AddProxyComparisonMethods {
	template<typename T, typename V>
	void operator()(pybind11::class_<Proxy<T, V>> &cl) {
		using P = Proxy<T, V>;
		cl
		.def("__lt__", [](P &p, V &val) {
			return p < makeConstantProxy<T>(val);
		})
		.def("__le__", [](P &p, V &val) {
			return p <= makeConstantProxy<T>(val);
		})
		.def("__gt__", [](P &p, V &val) {
			return p > makeConstantProxy<T>(val);
		})
		.def("__ge__", [](P &p, V &val) {
			return p >= makeConstantProxy<T>(val);
		})
		.def("__lt__", [](P &p1, P &p2) {
			return p1 < p2;
		})
		.def("__le__", [](P &p1, P &p2) {
			return p1 <= p2;
		})
		.def("__gt__", [](P &p1, P &p2) {
			return p1 > p2;
		})
		.def("__ge__", [](P &p1, P &p2) {
			return p1 >= p2;
		})
		;
	}
};

#endif /* proxy_comparison_hpp */