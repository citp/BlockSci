//
//  optional_arithmetic.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_optional_arithmetic_hpp
#define proxy_optional_arithmetic_hpp

#include "proxy.hpp"

void addProxyOptionalArithMethods(pybind11::class_<Proxy<ranges::optional<int64_t>>> &cl) {
	using P = Proxy<ranges::optional<int64_t>>;
	cl
	.def("__add__", [](P &p1, P &p2) -> P {
		return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
			auto v1 = p1(t);
			auto v2 = p2(t);
			if (v1 && v2) {
				return *v1 + *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__sub__", [](P &p1, P &p2) -> P {
		return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
			auto v1 = p1(t);
			auto v2 = p2(t);
			if (v1 && v2) {
				return *v1 - *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__mul__", [](P &p1, P &p2) -> P {
		return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
			auto v1 = p1(t);
			auto v2 = p2(t);
			if (v1 && v2) {
				return *v1 * *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__floordiv__", [](P &p1, P &p2) -> P {
		return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
			auto v1 = p1(t);
			auto v2 = p2(t);
			if (v1 && v2) {
				return *v1 / *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	.def("__mod__", [](P &p1, P &p2) -> P {
		return std::function<ranges::optional<int64_t>(std::any &)>{[=](std::any &t) -> ranges::optional<int64_t> {
			auto v1 = p1(t);
			auto v2 = p2(t);
			if (v1 && v2) {
				return *v1 % *v2;
			} else {
				return ranges::nullopt;
			}
		}};
	})
	;
}

#endif /* proxy_optional_arithmetic_hpp */