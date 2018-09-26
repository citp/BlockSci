//
//  arithmetic_range.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_arithmetic_range_hpp
#define proxy_arithmetic_range_hpp

struct AddProxyArithRangeMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T, int64_t>> &cl) {
		using P = Proxy<T, int64_t>;
		using P2 = Proxy<T, ranges::optional<int64_t>>;
		cl
		.def_property_readonly("min", [](P &p) -> int64_t {
			return p;
		})
		.def_property_readonly("max", [](P &p) -> int64_t {
			return p;
		})
		.def("__sub__", [](P &p, int64_t val) {
			return p - val;
		})
		.def("__mul__", [](P &p, int64_t val) {
			return p * val;
		})
		.def("__floordiv__", [](P &p, int64_t val) {
			return p / val;
		})
		.def("__mod__", [](P &p, int64_t val) {
			return p % val;
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
			return std::function<ranges::optional<int64_t>(T &)>{[=](T &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) + *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__sub__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(T &)>{[=](T &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) - *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__mul__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(T &)>{[=](T &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) * *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__floordiv__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(T &)>{[=](T &t) -> ranges::optional<int64_t> {
				auto v2 = p2(t);
				if (v2) {
					return p1(t) / *v2;
				} else {
					return ranges::nullopt;
				}
			}};
		})
		.def("__mod__", [](P &p1, P2 &p2) -> P2 {
			return std::function<ranges::optional<int64_t>(T &)>{[=](T &t) -> ranges::optional<int64_t> {
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

#endif /* proxy_arithmetic_range_hpp */