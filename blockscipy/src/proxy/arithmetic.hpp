//
//  arithmetic.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_arithmetic_hpp
#define proxy_arithmetic_hpp

#include "proxy.hpp"

void addProxyArithMethods(pybind11::class_<Proxy<int64_t>> &cl) {
	using P = Proxy<int64_t>;
	using P2 = Proxy<ranges::optional<int64_t>>;
	cl
	.def("__add__", [](P &p1, P &p2) -> P {
		return std::function<int64_t(std::any &)>{[=](std::any &t) -> int64_t {
			return p1(t) + p2(t);
		}};
	})
	.def("__sub__", [](P &p1, P &p2) -> P {
		return std::function<int64_t(std::any &)>{[=](std::any &t) -> int64_t {
			return p1(t) - p2(t);
		}};
	})
	.def("__mul__", [](P &p1, P &p2) -> P {
		return std::function<int64_t(std::any &)>{[=](std::any &t) -> int64_t {
			return p1(t) * p2(t);
		}};
	})
	.def("__floordiv__", [](P &p1, P &p2) -> P {
		return std::function<int64_t(std::any &)>{[=](std::any &t) -> int64_t {
			return p1(t) / p2(t);
		}};
	})
	.def("__mod__", [](P &p1, P &p2) -> P {
		return std::function<int64_t(std::any &)>{[=](std::any &t) -> int64_t {
			return p1(t) % p2(t);
		}};
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

#endif /* proxy_arithmetic_hpp */