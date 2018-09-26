//
//  proxy_operators.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_operators_hpp
#define proxy_operators_hpp

#include "proxy.hpp"
#include "range_conversion.hpp"

#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>

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

	struct map {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const -> decltype(convertRangeToPython(std::forward<A>(a) | ranges::view::transform(std::forward<B>(b)))) {
			return convertRangeToPython(std::forward<A>(a) | ranges::view::transform(std::forward<B>(b)));
		}
	};

	struct optionalHasValue {
		template <typename A>
		auto operator()(A && range) -> decltype(convertRangeToPython(std::forward<A>(range) | ranges::view::transform(&ranges::range_value_type_t<A>::has_value))) {
			return convertRangeToPython(range | ranges::view::transform(&ranges::range_value_type_t<A>::has_value));
		}
	};

	struct optionalWithValue {
		template <typename A>
		auto operator()(A && range) {
			return convertRangeToPython(std::forward<A>(range) | ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
	            | ranges::view::transform([](const auto &optional) { return *optional; }));
		}
	};

	struct optionalWithDefaultValue {
		template <typename A, typename B>
		auto operator()(A && range, B && val) {
			return convertRangeToPython(std::forward<A>(range) | ranges::view::transform([=](const auto &optional) {
                if (optional) {
                    return *optional;
                } else {
                    return val;
                }
            }));
		}
	};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator==(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<bool(T1 &)>{compose(p.func, curry(internal::eq{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, bool> operator==(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::eq{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator!=(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<bool(T1 &)>{compose(p.func, curry(internal::neq{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, bool> operator!=(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::neq{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator>(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<bool(T1 &)>{compose(p.func, curry(internal::gt{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, bool> operator>(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::gt{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator>=(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<bool(T1 &)>{compose(p.func, curry(internal::gte{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, bool> operator>=(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::gte{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator<(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<bool(T1 &)>{compose(p.func, curry(internal::lt{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, bool> operator<(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::lt{})};
}

template <typename T1, typename T2>
Proxy<T1, bool> operator<=(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<bool(T1 &)>{compose(p.func, curry(internal::lte{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, bool> operator<=(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<bool(T1 &)>{compose2(p1.func, p2.func, internal::lte{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator+(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<T2(T1 &)>{compose(p.func, curry(internal::add{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, T2> operator+(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::add{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator-(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<T2(T1 &)>{compose(p.func, curry(internal::sub{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, T2> operator-(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::sub{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator*(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<T2(T1 &)>{compose(p.func, curry(internal::mul{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, T2> operator*(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::mul{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator/(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<T2(T1 &)>{compose(p.func, curry(internal::div{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, T2> operator/(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::div{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator%(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<T2(T1 &)>{compose(p.func, curry(internal::mod{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, T2> operator%(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::mod{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator&&(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<T2(T1 &)>{compose(p.func, curry(internal::_and{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, T2> operator&&(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::_and{})};
}

template <typename T1, typename T2>
Proxy<T1, T2> operator||(const Proxy<T1, T2> &p, const T2 &val) {
	return std::function<T2(T1 &)>{compose(p.func, curry(internal::_or{}, val))};
}
template <typename T1, typename T2>
Proxy<T1, T2> operator||(const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) {
	return std::function<T2(T1 &)>{compose2(p1.func, p2.func, internal::_or{})};
}

#endif /* proxy_operators_hpp */