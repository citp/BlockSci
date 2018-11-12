//
//  arithmetic.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_arithmetic_hpp
#define proxy_arithmetic_hpp

#include "proxy.hpp"

template<typename Class>
void addProxyArithMethods(Class &cl) {
	using P = typename Class::type;
	using T = typename P::output_t;
	using P2 = Proxy<ranges::optional<T>>;
	cl
	.def("__add__", [](P &p1, P &p2) -> P {
		return std::function<T(std::any &)>{[=](std::any &t) -> T {
			return p1(t) + p2(t);
		}};
	})
	.def("__sub__", [](P &p1, P &p2) -> P {
		return std::function<T(std::any &)>{[=](std::any &t) -> T {
			return p1(t) - p2(t);
		}};
	})
	.def("__mul__", [](P &p1, P &p2) -> P {
		return std::function<T(std::any &)>{[=](std::any &t) -> T {
			return p1(t) * p2(t);
		}};
	})
	.def("__floordiv__", [](P &p1, P &p2) -> P {
		return std::function<T(std::any &)>{[=](std::any &t) -> T {
			return p1(t) / p2(t);
		}};
	})
	.def("__mod__", [](P &p1, P &p2) -> P {
		return std::function<T(std::any &)>{[=](std::any &t) -> T {
			return p1(t) % p2(t);
		}};
	})

	.def("__add__", [](P &p1, P2 &p2) -> P2 {
		return std::function<ranges::optional<T>(std::any &)>{[=](std::any &t) -> ranges::optional<T> {
			auto v2 = p2(t);
			if (v2) {
				return p1(t) + *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__sub__", [](P &p1, P2 &p2) -> P2 {
		return std::function<ranges::optional<T>(std::any &)>{[=](std::any &t) -> ranges::optional<T> {
			auto v2 = p2(t);
			if (v2) {
				return p1(t) - *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__mul__", [](P &p1, P2 &p2) -> P2 {
		return std::function<ranges::optional<T>(std::any &)>{[=](std::any &t) -> ranges::optional<T> {
			auto v2 = p2(t);
			if (v2) {
				return p1(t) * *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__floordiv__", [](P &p1, P2 &p2) -> P2 {
		return std::function<ranges::optional<T>(std::any &)>{[=](std::any &t) -> ranges::optional<T> {
			auto v2 = p2(t);
			if (v2) {
				return p1(t) / *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__mod__", [](P &p1, P2 &p2) -> P2 {
		return std::function<ranges::optional<T>(std::any &)>{[=](std::any &t) -> ranges::optional<T> {
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

#endif /* proxy_arithmetic_hpp */