//
//  optional_range.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_optional_range_hpp
#define proxy_optional_range_hpp

#include "proxy_py.hpp"
#include "proxy_utils.hpp"
#include "optional_utils.hpp"
#include "simplify_range.hpp"

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/distance.hpp>

template<typename Range, typename OutType>
using map_optional_proxy_type = Proxy<typename ranges::range_value_type_t<Range>::value_type, OutType>;

template<typename Range, typename ProxyType>
using map_optional_result_type = decltype(simplifyRange(ranges::view::transform(std::declval<Range>(), std::declval<ProxyType>())));

template<typename T2, typename T3>
struct MapOptionalProxyFunctor {
	using RawFunc2 = map_optional_proxy_type<T2, T3>;
	using Func2 = MakeOptionalFunctor<RawFunc2>;
	using ResultType = map_optional_result_type<T2, Func2>;
	Func2 p2;

	MapOptionalProxyFunctor(RawFunc2 &p2_) : p2(p2_) {}
	ResultType operator()(T2 && val) const {
		return simplifyRange(ranges::view::transform(std::move(val), p2));
	}
};

template<typename T1, typename T2, typename T3>
auto mapOptionalProxy(Proxy<T1, T2> &p1, map_optional_proxy_type<T2, T3> &p2) -> Proxy<T1, decltype(MapOptionalProxyFunctor<T2, T3>{p2}(std::declval<T2>()))> {
	return lift(p1, MapOptionalProxyFunctor<T2, T3>{p2});
}

template<typename R, typename T, typename V>
void addProxyOptionalMapFunc(pybind11::class_<Proxy<T, V>> &cl) {
    cl
    .def("map", mapOptionalProxy<T, V, R> )
    .def("map", mapOptionalProxy<T, V, ranges::optional<R>>)
    ;
}

struct AddProxyOptionalRangeMethods {
	template<typename T, typename V, ranges::category range_cat>
	void operator()(pybind11::class_<Proxy<T, ranges::any_view<ranges::optional<V>, range_cat>>> &cl) {
		using P = Proxy<T, ranges::any_view<ranges::optional<V>, range_cat>>;

		cl.def_property_readonly("size", [](P &p) -> Proxy<T, int64_t> {
			return lift(p, [](ranges::any_view<ranges::optional<V>, range_cat> &&r) -> int64_t {
				return ranges::distance(r);
			});
		});

		cl.def_property_readonly("has_value", [](P &p) -> Proxy<T, ranges::any_view<bool, range_cat>> {
			return std::function<ranges::any_view<bool, range_cat>(T &)>{[=](T &t) {
				return ranges::any_view<bool, range_cat>{ranges::view::transform(p(t), &ranges::optional<V>::has_value)};
			}};
        }, "Return a array of bools denoting whether a item in the sequence has a value or is none");
        
        cl.def_property_readonly("with_value", [](P &p) -> Proxy<T, ranges::any_view<V>> {
        	return std::function<ranges::any_view<V>(T &)>{[=](T &t) -> ranges::any_view<V> {
				return ranges::view::transform(
					ranges::view::filter(p(t), [](const ranges::optional<V> &optional) {
						return static_cast<bool>(optional); 
					}), [](const ranges::optional<V> &optional) {
						return *optional;
					}
				);
			}};
        }, "Returns a sequence containing only non-None items in the sequence");

        cl.def("with_default_value", [](P &p, const V &defVal) -> Proxy<T, ranges::any_view<V, range_cat>> {
        	return std::function<ranges::any_view<V, range_cat>(T &)>{[=](T &t) -> ranges::any_view<V, range_cat> {
				return ranges::view::transform(p(t), [=](const ranges::optional<V> &optional) {
					if (optional) {
	                    return *optional;
	                } else {
	                    return defVal;
	                }
				});
			}};
        }, pybind11::arg("default_value"), "Replace all none values in the sequence with the provided default value and return the resulting sequence");

		addProxyOptionalMapFunc<blocksci::Block>(cl);
		addProxyOptionalMapFunc<blocksci::Transaction>(cl);
		addProxyOptionalMapFunc<blocksci::Input>(cl);
		addProxyOptionalMapFunc<blocksci::Output>(cl);
		addProxyOptionalMapFunc<blocksci::AnyScript>(cl);
		addProxyOptionalMapFunc<blocksci::AddressType::Enum>(cl);
		addProxyOptionalMapFunc<int64_t>(cl);
		addProxyOptionalMapFunc<bool>(cl);
		addProxyOptionalMapFunc<std::chrono::system_clock::time_point>(cl);
		addProxyOptionalMapFunc<blocksci::uint256>(cl);
		addProxyOptionalMapFunc<blocksci::uint160>(cl);
		addProxyOptionalMapFunc<pybind11::bytes>(cl);
	}
};

template <typename T>
void setupOptionalRangesProxy(AllProxyClasses<T> &cls) {
	cls.optionalIterator.applyToAll(AddProxyOptionalRangeMethods{});
	cls.optionalRange.applyToAll(AddProxyOptionalRangeMethods{});
}

#endif /* proxy_optional_range_hpp */
