//
//  optional.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/31/18.
//
//

#include "optional.hpp"
#include "optional_map.hpp"
#include "proxy.hpp"
#include "proxy_utils.hpp"
#include "caster_py.hpp"

void addOptionalProxyMethods(pybind11::class_<OptionalProxy, GenericProxy> &cl, pybind11::module &m) {
	addOptionalProxyMapMethods(cl);
	
	cl
	.def("has_value", [](OptionalProxy &p) -> Proxy<bool> {
		return liftGeneric(p, [](auto && val) {
			return std::forward<decltype(val)>(val).has_value();
		});
	})
	;

	m
    .def("take_while", [](OptionalProxy &body, BlocksciTypeVariant &init) -> pybind11::list {
        auto generic = body.getGeneric();
        pybind11::list vec;
        ranges::optional<BlocksciType> v = BlocksciType{init};
        while(true) {

        	addToList(vec, *v);
        	auto anyVal = v->toAny();
        	auto nextVal = generic(anyVal);
        	if (nextVal) {
        		v.emplace(*nextVal);
        	} else {
        		break;
        	}
        }
        return vec;
    })
	;
}

