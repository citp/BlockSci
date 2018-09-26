//
//  optional.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_optional_hpp
#define proxy_optional_hpp

template<typename T1, typename T2, typename T3>
auto mapOptional(Proxy<T1, ranges::optional<T2>> &p1, Proxy<T2, T3> &p2) -> Proxy<T1, ranges::optional<T3>> {
	return std::function<ranges::optional<T3>(T1 &)>{
		[=](T1 &val) -> ranges::optional<T3> {
			auto v = p1(val);
			if (v) {
				return p2(*v);
			} else {
				return ranges::nullopt;
			}
		}
	};
}



struct AddProxyOptionalMethods {
	template<typename T, typename V>
	void operator()(pybind11::class_<Proxy<T, ranges::optional<V>>> &cl) {
		using P = Proxy<T, ranges::optional<V>>;

		cl
		.def("or_value", [](P &p, V &v) -> Proxy<T, V> {
			return std::function<V(T &)>{[=](T &t) {
				auto opt = p(t);
				if (opt) {
					return *opt;
				} else {
					return v;
				}
			}};
		})
		.def("has_value", [](P &p) -> Proxy<T, bool> {
			return std::function<bool(T &)>{[=](T &t) -> bool {
				return p(t).has_value();
			}};
		})
		;
	}
};

struct AddProxyOptionalMapMethods {
	template<typename T, typename V>
	void operator()(pybind11::class_<Proxy<T, ranges::optional<V>>> &cl) {
		using P = Proxy<T, ranges::optional<V>>;

		cl
		.def("map", [](P &p, Proxy<V, blocksci::Block> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, blocksci::Transaction> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, blocksci::Input> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, blocksci::Output> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, blocksci::AnyScript> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, blocksci::AddressType::Enum> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, int64_t> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, bool> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, std::chrono::system_clock::time_point> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, blocksci::uint256> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, blocksci::uint160> &p2) {
			return mapOptional(p, p2);
		})
		.def("map", [](P &p, Proxy<V, pybind11::bytes> &p2) {
			return mapOptional(p, p2);
		})
		;
	}
};

#endif /* proxy_optional_hpp */
