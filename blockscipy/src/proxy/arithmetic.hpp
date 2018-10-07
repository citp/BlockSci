//
//  arithmetic.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_arithmetic_hpp
#define proxy_arithmetic_hpp

#include "proxy.hpp"

namespace internal {
	struct add {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(a + b) {
			return std::forward<A>(a) + std::forward<B>(b);
		}
	};

	struct sub {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(a - b) {
			return std::forward<A>(a) - std::forward<B>(b);
		}
	};

	struct mul {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(a * b) {
			return std::forward<A>(a) * std::forward<B>(b);
		}
	};

	struct div {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(a / b) {
			return std::forward<A>(a) / std::forward<B>(b);
		}
	};

	struct mod {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(a % b) {
			return std::forward<A>(a) % std::forward<B>(b);
		}
	};
}

template <typename T>
Proxy<T> operator+(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<T(std::any &)>{compose2(p1.func, p2.func, internal::add{})};
}

template <typename T>
Proxy<T> operator-(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<T(std::any &)>{compose2(p1.func, p2.func, internal::sub{})};
}

template <typename T>
Proxy<T> operator*(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<T(std::any &)>{compose2(p1.func, p2.func, internal::mul{})};
}

template <typename T>
Proxy<T> operator/(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<T(std::any &)>{compose2(p1.func, p2.func, internal::div{})};
}

template <typename T>
Proxy<T> operator%(const Proxy<T> &p1, const Proxy<T> &p2) {
	return std::function<T(std::any &)>{compose2(p1.func, p2.func, internal::mod{})};
}

struct AddProxyArithMethods {
	void operator()(pybind11::class_<Proxy<int64_t>> &cl) {
		using P = Proxy<int64_t>;
		using P2 = Proxy<ranges::optional<int64_t>>;
		cl
		.def("__add__", [](P &p, int64_t val) {
			return p + makeConstantProxy(val);
		})
		.def("__sub__", [](P &p, int64_t val) {
			return p - makeConstantProxy(val);
		})
		.def("__mul__", [](P &p, int64_t val) {
			return p * makeConstantProxy(val);
		})
		.def("__floordiv__", [](P &p, int64_t val) {
			return p / makeConstantProxy(val);
		})
		.def("__mod__", [](P &p, int64_t val) {
			return p % makeConstantProxy(val);
		})

		.def("__add__", [](P &p1, P &p2) {
			return p1 + p2;
		})
		.def("__sub__", [](P &p1, P &p2) {
			return p1 - p2;
		})
		.def("__mul__", [](P &p1, P &p2) {
			return p1 * p2;
		})
		.def("__floordiv__", [](P &p1, P &p2) {
			return p1 / p2;
		})
		.def("__mod__", [](P &p1, P &p2) {
			return p1 % p2;
		})

		.def("__add__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) + *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__sub__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) - *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__mul__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) * *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__floordiv__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) / *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__mod__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) % *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		;
	}
};

#endif /* proxy_arithmetic_hpp */