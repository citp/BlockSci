//
//  optional.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_optional_hpp
#define proxy_optional_hpp

#include "proxy.hpp"

template<typename T1, typename T2>
auto mapOptional(Proxy<ranges::optional<T1>> &p1, Proxy<T2> &p2) -> Proxy<ranges::optional<T2>> {
	return std::function<ranges::optional<T2>(std::any &)>{
		[=](std::any &val) -> ranges::optional<T2> {
			auto v = p1(val);
			if (v) {
				return p2(*v);
			} else {
				return ranges::nullopt;
			}
		}
	};
}



template<typename T>
void addProxyOptionalMethods(pybind11::class_<Proxy<ranges::optional<T>>> &cl) {
	using P = Proxy<ranges::optional<T>>;

	cl
	.def("or_value", [](P &p, T &v) -> Proxy<T> {
		return std::function<T(std::any &)>{[=](std::any &t) {
			auto opt = p(t);
			if (opt) {
				return *opt;
			} else {
				return v;
			}
		}};
	})
	.def("has_value", [](P &p) -> Proxy<bool> {
		return std::function<bool(std::any &)>{[=](std::any &t) -> bool {
			return p(t).has_value();
		}};
	})
	;
}

template<typename T>
void addProxyOptionalMapMethods(pybind11::class_<Proxy<ranges::optional<T>>> &cl) {
	using P = Proxy<ranges::optional<T>>;

	cl
	.def("map", [](P &p, Proxy<blocksci::Block> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<blocksci::Transaction> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<blocksci::Input> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<blocksci::Output> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<blocksci::AnyScript> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<blocksci::AddressType::Enum> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<int64_t> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<bool> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<std::chrono::system_clock::time_point> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<blocksci::uint256> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<blocksci::uint160> &p2) {
		return mapOptional(p, p2);
	})
	.def("map", [](P &p, Proxy<pybind11::bytes> &p2) {
		return mapOptional(p, p2);
	})
	;
}

#endif /* proxy_optional_hpp */
