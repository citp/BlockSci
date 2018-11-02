//
//  arithmetic_range.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_arithmetic_range_hpp
#define proxy_arithmetic_range_hpp

#include "proxy.hpp"

#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/min.hpp>
#include <range/v3/numeric/accumulate.hpp>

template<typename Class>
void addProxyArithRangeMethods(Class  &cl) {
	using P = typename Class::type;
	using T = typename P::output_t;
	cl
	.def_property_readonly("min", [](P &p) -> Proxy<int64_t> {
		return lift(p, [](T &&r) {
			return ranges::min(std::move(r));
		});
	})
	.def_property_readonly("max", [](P &p) -> Proxy<int64_t> {
		return lift(p, [](T &&r) {
			return ranges::max(std::move(r));
		});
	})
	.def_property_readonly("sum", [](P &p) -> Proxy<int64_t> {
		return lift(p, [](T &&r) {
			return ranges::accumulate(std::move(r), int64_t(0));
		});
	})
	;
}

#endif /* proxy_arithmetic_range_hpp */