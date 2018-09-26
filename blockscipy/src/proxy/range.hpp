//
//  range.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_hpp
#define proxy_range_hpp

#include "proxy_py.hpp"
#include "proxy_utils.hpp"
#include "simplify_range.hpp"

#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/distance.hpp>

template<typename Range, typename OutType>
using map_proxy_type = Proxy<ranges::range_value_type_t<Range>, OutType>;

template<typename Range, typename ProxyType>
using map_result_type = decltype(simplifyRange(ranges::view::transform(std::declval<Range>(), std::declval<ProxyType>())));

template<typename T2, typename T3>
struct MapProxyFunctor {
	using Func2 = map_proxy_type<T2, T3>;
	using ResultType = map_result_type<T2, Func2>;
	Func2 p2;

	MapProxyFunctor(Func2 &p2_) : p2(p2_) {}

	ResultType operator()(T2 && val) const {
		return simplifyRange(ranges::view::transform(std::move(val), p2));
	}
};

template<typename T1, typename T2, typename T3>
auto mapProxy(Proxy<T1, T2> &p1, map_proxy_type<T2, T3> &p2) -> Proxy<T1, decltype(MapProxyFunctor<T2, T3>{p2}(std::declval<T2>()))> {
	return lift(p1, MapProxyFunctor<T2, T3>{p2});
}

template<typename R, typename T, typename V>
void addProxyMapFunc(pybind11::class_<Proxy<T, V>> &cl) {
    cl
    .def("map", mapProxy<T, V, R>)
    .def("map", mapProxy<T, V, ranges::optional<R>>)
    ;
}

struct AddProxyRangeMethods {
	template<typename T, typename V, ranges::category range_cat>
	void operator()(pybind11::class_<Proxy<T, ranges::any_view<V, range_cat>>> &cl) {
		using P = Proxy<T, ranges::any_view<V, range_cat>>;

		cl
		.def("any", [](P &p, Proxy<V, bool> &p2) -> Proxy<T, bool> {
			return lift(p, [=](ranges::any_view<V, range_cat> && val) -> bool {
				return ranges::any_of(val, [=](V item) {
					return p2(item);
				});
			});
		})
		.def("all", [](P &p, Proxy<V, bool> &p2) -> Proxy<T, bool> {
			return lift(p, [=](ranges::any_view<V, range_cat> && val) -> bool {
				return ranges::all_of(val, [=](V item) {
					return p2(item);
				});
			});
		})
		.def_property_readonly("size", [](P &p) -> Proxy<T, int64_t> {
			return lift(p, [](ranges::any_view<ranges::optional<V>, range_cat> &&r) -> int64_t {
				return ranges::distance(r);
			});
		})
		;

		addProxyMapFunc<blocksci::Block>(cl);
		addProxyMapFunc<blocksci::Transaction>(cl);
		addProxyMapFunc<blocksci::Input>(cl);
		addProxyMapFunc<blocksci::Output>(cl);
		addProxyMapFunc<blocksci::AnyScript>(cl);
		addProxyMapFunc<blocksci::AddressType::Enum>(cl);
		addProxyMapFunc<int64_t>(cl);
		addProxyMapFunc<bool>(cl);
		addProxyMapFunc<std::chrono::system_clock::time_point>(cl);
		addProxyMapFunc<blocksci::uint256>(cl);
		addProxyMapFunc<blocksci::uint160>(cl);
		addProxyMapFunc<pybind11::bytes>(cl);
	}
};

template <typename T>
void setupRangesProxy(AllProxyClasses<T> &cls) {
	cls.iterator.applyToAll(AddProxyRangeMethods{});
	cls.range.applyToAll(AddProxyRangeMethods{});
}

#endif /* proxy_range_hpp */