//
//  range_methods.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/1/18.
//
//

#include "range_map.hpp"

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/all_of.hpp>

void addProxyIteratorMethods(pybind11::class_<IteratorProxy> &cl) {
	cl
	.def_property_readonly("size", [](IteratorProxy &seq) -> Proxy<int64_t> {
		auto generic = seq.getGeneric();
		return std::function<int64_t(std::any &)>{[=](std::any &val) -> int64_t {
			return ranges::distance(generic(val));
		}};
	})
	.def("_any", [](IteratorProxy &seq, Proxy<bool> &p2) -> Proxy<bool> {
		auto generic = seq.getGeneric();
		return std::function<bool(std::any &)>{[=](std::any &val) -> bool {
			return ranges::any_of(generic(val), [=](const std::any &item) {
				auto r = item;
				return p2(r);
			});
		}};
	})
	.def("_all", [](IteratorProxy &seq, Proxy<bool> &p2) -> Proxy<bool> {
		auto generic = seq.getGeneric();
		return std::function<bool(std::any &)>{[=](std::any &val) -> bool {
			return ranges::all_of(generic(val), [=](const std::any &item) {
				auto r = item;
				return p2(r);
			});
		}};
	})
	;
}

void addProxyRangeMethods(pybind11::class_<RangeProxy> &cl) {
	cl
	.def_property_readonly("size", [](RangeProxy &seq) -> Proxy<int64_t> {
		auto generic = seq.getGeneric();
		return std::function<int64_t(std::any &)>{[=](std::any &val) -> int64_t {
			return ranges::distance(generic(val));
		}};
	})
	;
}

