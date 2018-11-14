//
//  range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/1/18.
//
//

#include "range.hpp"
#include "range_map.hpp"
#include "proxy.hpp"
#include "proxy_utils.hpp"

#include <range/v3/distance.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/all_of.hpp>

void applyProxyIteratorFuncs(pybind11::class_<IteratorProxy, GenericProxy> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);

    applyProxyMapOptionalFuncsCore(cl);
	applyProxyMapOptionalFuncsScripts(cl);
	applyProxyMapOptionalFuncsOther(cl);

    applyProxyMapSequenceFuncsCore(cl);
    applyProxyMapSequenceFuncsScripts(cl);
    applyProxyMapSequenceFuncsOther(cl);

    cl
	.def_property_readonly("size", [](IteratorProxy &p) -> Proxy<int64_t> {
		return liftGeneric(p, [](auto && seq) -> int64_t {
			return ranges::distance(std::forward<decltype(seq)>(seq));
		});
	})
	.def("_any", [](IteratorProxy &p, Proxy<bool> &p2) -> Proxy<bool> {
		return liftGeneric(p, [p2](auto && seq) -> bool {
			return ranges::any_of(std::forward<decltype(seq)>(seq), [p2](const BlocksciType &item) {
				return p2(item.toAny());
			});
		});
	})
	.def("_all", [](IteratorProxy &p, Proxy<bool> &p2) -> Proxy<bool> {
		return liftGeneric(p, [p2](auto && seq) -> bool {
			return ranges::all_of(std::forward<decltype(seq)>(seq), [p2](const BlocksciType &item) {
				return p2(item.toAny());
			});
		});
	})
	;
}

void applyProxyRangeFuncs(pybind11::class_<RangeProxy, IteratorProxy> &cl) {
	applyProxyMapFuncsCore(cl);
	applyProxyMapFuncsScripts(cl);
	applyProxyMapFuncsOther(cl);

    cl
	.def_property_readonly("size", [](RangeProxy &p) -> Proxy<int64_t> {
		return liftGeneric(p, [](auto && seq) -> int64_t {
			return ranges::distance(std::forward<decltype(seq)>(seq));
		});
	})
	;
}