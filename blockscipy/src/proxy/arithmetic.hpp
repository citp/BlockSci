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
		return lift(p1, p2, [](auto && v1, auto && v2) -> T {
			return std::forward<decltype(v1)>(v1) + std::forward<decltype(v2)>(v2);
		});
	})
	.def("__sub__", [](P &p1, P &p2) -> P {
		return lift(p1, p2, [](auto && v1, auto && v2) -> T {
			return std::forward<decltype(v1)>(v1) - std::forward<decltype(v2)>(v2);
		});
	})
	.def("__mul__", [](P &p1, P &p2) -> P {
		return lift(p1, p2, [](auto && v1, auto && v2) -> T {
			return std::forward<decltype(v1)>(v1) * std::forward<decltype(v2)>(v2);
		});
	})
	.def("__floordiv__", [](P &p1, P &p2) -> P {
		return lift(p1, p2, [](auto && v1, auto && v2) -> T {
			return std::forward<decltype(v1)>(v1) / std::forward<decltype(v2)>(v2);
		});
	})
	.def("__mod__", [](P &p1, P &p2) -> P {
		return lift(p1, p2, [](auto && v1, auto && v2) -> T {
			return std::forward<decltype(v1)>(v1) % std::forward<decltype(v2)>(v2);
		});
	})

	.def("__add__", [](P &p1, P2 &p2) -> P2 {
		return lift(p1, p2, [](auto && v1, auto && v2) -> ranges::optional<T> {
			if (v2) {
				return std::forward<decltype(v1)>(v1) + *v2;
			} else {
				return ranges::nullopt;
			}
		});
	})
	.def("__sub__", [](P &p1, P2 &p2) -> P2 {
		return lift(p1, p2, [](auto && v1, auto && v2) -> ranges::optional<T> {
			if (v2) {
				return std::forward<decltype(v1)>(v1) - *v2;
			} else {
				return ranges::nullopt;
			}
		});
	})
	.def("__mul__", [](P &p1, P2 &p2) -> P2 {
		return lift(p1, p2, [](auto && v1, auto && v2) -> ranges::optional<T> {
			if (v2) {
				return std::forward<decltype(v1)>(v1) * *v2;
			} else {
				return ranges::nullopt;
			}
		});
	})
	.def("__floordiv__", [](P &p1, P2 &p2) -> P2 {
		return lift(p1, p2, [](auto && v1, auto && v2) -> ranges::optional<T> {
			if (v2) {
				return std::forward<decltype(v1)>(v1) / *v2;
			} else {
				return ranges::nullopt;
			}
		});
	})
	.def("__mod__", [](P &p1, P2 &p2) -> P2 {
		return lift(p1, p2, [](auto && v1, auto && v2) -> ranges::optional<T> {
			if (v2) {
				return std::forward<decltype(v1)>(v1) % *v2;
			} else {
				return ranges::nullopt;
			}
		});
	})
	;
}

#endif /* proxy_arithmetic_hpp */