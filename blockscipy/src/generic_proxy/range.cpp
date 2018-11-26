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
#include "caster_py.hpp"

#include <range/v3/distance.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/all_of.hpp>

void applyProxyIteratorFuncs(pybind11::class_<IteratorProxy, GenericProxy> &cl, pybind11::module &m) {
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
			return mpark::visit(ranges::distance, std::forward<decltype(seq)>(seq).var);
		});
	})
	.def("_any", [](IteratorProxy &p, Proxy<bool> &p2) -> Proxy<bool> {
		return liftGeneric(p, [p2](auto && seq) -> bool {
			return mpark::visit([p2](auto && r) -> bool {
				return ranges::any_of(std::forward<decltype(r)>(r), [p2](auto && item) {
					return p2(std::forward<decltype(item)>(item));
				});
			}, std::forward<decltype(seq)>(seq).var);
			
		});
	})
	.def("_all", [](IteratorProxy &p, Proxy<bool> &p2) -> Proxy<bool> {
		return liftGeneric(p, [p2](auto && seq) -> bool {
			return mpark::visit([p2](auto && r) -> bool {
				return ranges::all_of(std::forward<decltype(r)>(r), [p2](auto && item) {
					return p2(std::forward<decltype(item)>(item));
				});
			}, std::forward<decltype(seq)>(seq).var);
			
		});
	})
	;

    m.def("_traverse", [](IteratorProxy &p, BlocksciTypeVariant &init) -> pybind11::list {
    	auto generic = p.getGeneric();

        std::vector<BlocksciType> in;
        pybind11::list out;
        in.emplace_back(init);
        while (!in.empty()) {
        	auto next = in.back();
        	addToList(out, next);
            in.pop_back();
            auto anyV = next.toAny();
            mpark::visit([&in](auto && seq) {
            	for (auto && elem : std::forward<decltype(seq)>(seq)) {
            		in.emplace_back(std::forward<decltype(elem)>(elem));
            	}
            }, generic(anyV).var);
        }
        return out;
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
			return mpark::visit(ranges::distance, std::forward<decltype(seq)>(seq).var);
		});
	})
	;
}