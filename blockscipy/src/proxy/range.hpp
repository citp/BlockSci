//
//  range.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_hpp
#define proxy_range_hpp

#include "proxy_py.hpp"
#include "proxy_operators.hpp"

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/all_of.hpp>

template <typename GroupType, typename ValueType>
using ProxyGroup = std::unordered_map<GroupType, std::vector<ValueType>>;


template<typename T1, typename T2, typename T3>
auto groupProxy(Proxy<T1, T2> &p1, Proxy<ranges::range_value_type_t<T2>, T3> &p2) -> Proxy<T1, ProxyGroup<T3, ranges::range_value_type_t<T2>>> {
	return std::function<ProxyGroup<T3, ranges::range_value_type_t<T2>>(T1 &)>{[=](T1 &val) {
		ProxyGroup<T3, ranges::range_value_type_t<T2>> grouping;
		for (const auto & v : p1(val)) {
			grouping[p2(v)].push_back(v);
		}
		return grouping;
	}};
}

template<typename T1, typename T2, typename T3>
auto mapProxy(Proxy<T1, T2> &p1, Proxy<ranges::range_value_type_t<T2>, T3> &p2) -> Proxy<T1, decltype(internal::map{}(std::declval<T2>(), p2))> {
	return std::function<decltype(internal::map{}(std::declval<T2>(), p2))(T1 &)>{
		[=](T1 &val) {
			return internal::map{}(p1(val), p2);
		}
	};
}

namespace internal {
	struct map_optional {
		template <typename A, typename B>
		auto operator()(A && a, B && b) const {
			return convertRangeToPython(ranges::view::transform(std::forward<A>(a), [=](auto && optional) -> ranges::optional<decltype(b(*optional))> {
	            if (optional) {
	                return b(*optional);
	            } else {
	                return ranges::nullopt;
	            }
	        }));
		}
	};
}

template<typename T1, typename T2, typename T3>
auto mapOptionalProxy(Proxy<T1, T2> &p1, Proxy<typename ranges::range_value_type_t<T2>::value_type, T3> &p2) -> Proxy<T1, decltype(internal::map_optional{}(std::declval<T2>(), p2))> {
	return std::function<decltype(internal::map_optional{}(std::declval<T2>(), p2))(T1 &)>{[=](T1 &val) {
		return internal::map_optional{}(p1(val), p2);
	}};
}

template<typename R, typename T, typename V>
void addProxyMapFunc(pybind11::class_<Proxy<T, V>> &cl) {
	using P = Proxy<T, V>;
    using range_type = ranges::range_value_type_t<V>;
    cl
    .def("map", [](P &p, Proxy<range_type, R> &p2) {
    	return mapProxy(p, p2);
    })
    .def("map", [](P &p, Proxy<range_type, ranges::optional<R>> &p2) {
        return mapProxy(p, p2);
    })
    ;
}

template<typename R, typename T, typename V>
void addProxyOptionalMapFunc(pybind11::class_<Proxy<T, V>> &cl) {
	using P = Proxy<T, V>;
    using range_type = ranges::range_value_type_t<V>;
    using value_type = typename range_type::value_type;
    cl
    .def("map", [](P &p, Proxy<value_type, R> &p2) {
    	return mapOptionalProxy(p, p2);
    })
    .def("map", [](P &p, Proxy<value_type, ranges::optional<R>> &p2) {
        return mapOptionalProxy(p, p2);
    })
    ;
}



struct AddProxyRangeMethods {
	template<typename T, typename V>
	void operator()(pybind11::class_<Proxy<T, V>> &cl) {
		using P = Proxy<T, V>;
		using range_type = ranges::range_value_type_t<V>;

		cl
		.def("any", [](P &p, Proxy<range_type, bool> &p2) -> Proxy<T, bool> {
			return std::function<bool(T &)>{[=](T &val) -> bool {
				auto range = p(val);
				return ranges::any_of(range, [=](auto item) {
					auto it = item;
					return p2(it);
				});
			}};
		})
		.def("all", [](P &p, Proxy<range_type, bool> &p2) -> Proxy<T, bool> {
			return std::function<bool(T &)>{[=](T &val) -> bool {
				auto range = p(val);
				return ranges::all_of(range, [=](auto item) {
					auto it = item;
					return p2(it);
				});
			}};
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

struct AddProxyOptionalRangeMethods {
	template<typename T, typename V>
	void operator()(pybind11::class_<Proxy<T, V>> &cl) {
		using P = Proxy<T, V>;
		using range_type = ranges::range_value_type_t<V>;
		using value_type = typename range_type::value_type;
		constexpr auto range_cat = getBlockSciCategory(ranges::get_categories<V>());

		cl.def_property_readonly("has_value", [](P &p) -> Proxy<T, ranges::any_view<bool, range_cat>> {
			return std::function<ranges::any_view<bool, range_cat>(T &)>{[=](T &t) {
				return ranges::any_view<bool, range_cat>{p(t) | ranges::view::transform(&range_type::has_value)};
			}};
        }, "Return a array of bools denoting whether a item in the sequence has a value or is none");
        
        cl.def_property_readonly("with_value", [](P &p) {
        	return [=](T &t) -> ranges::any_view<value_type> {
				return p(t) | 
				ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); }) |
				ranges::view::transform([](const auto &optional) { return *optional; });
			};
        }, "Returns a sequence containing only non-None items in the sequence");

        cl.def("with_default_value", [](P &p, const value_type &defVal) {
        	return [=](T &t) -> ranges::any_view<value_type> {
				return p(t) | ranges::view::transform([=](const auto &optional) {
					if (optional) {
	                    return *optional;
	                } else {
	                    return defVal;
	                }
				});
			};
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
void setupRangesProxy(AllProxyClasses<T> &cls) {
	cls.iterator.applyToAll(AddProxyRangeMethods{});
	cls.range.applyToAll(AddProxyRangeMethods{});
	cls.optionalIterator.applyToAll(AddProxyOptionalRangeMethods{});
	cls.optionalRange.applyToAll(AddProxyOptionalRangeMethods{});
}

#endif /* proxy_range_hpp */