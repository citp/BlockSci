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

template <typename T>
Proxy<bool> operator>(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<bool(std::any &)>{compose2(p1.func, p2.func, internal::gt{})};
}

template <typename T>
Proxy<bool> operator>=(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<bool(std::any &)>{compose2(p1.func, p2.func, internal::gte{})};
}

template <typename T>
Proxy<bool> operator<(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<bool(std::any &)>{compose2(p1.func, p2.func, internal::lt{})};
}

template <typename T>
Proxy<bool> operator<=(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<bool(std::any &)>{compose2(p1.func, p2.func, internal::lte{})};
}

struct AddProxyComparisonMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T>> &cl) {
		using P = Proxy<T>;
		cl
		.def("__lt__", [](P &p, T &val) {
			return p < makeConstantProxy(val);
		})
		.def("__le__", [](P &p, T &val) {
			return p <= makeConstantProxy(val);
		})
		.def("__gt__", [](P &p, T &val) {
			return p > makeConstantProxy(val);
		})
		.def("__ge__", [](P &p, T &val) {
			return p >= makeConstantProxy(val);
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