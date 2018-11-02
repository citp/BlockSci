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

template<typename T>
void addProxyArithRangeMethods(pybind11::class_<SequenceProxy<T>> &cl) {
	cl
	.def_property_readonly("min", [](SequenceProxy<T> &seq) -> Proxy<int64_t> {
		auto generic = seq.getIteratorFunc();
		return std::function<int64_t(std::any &)>{[=](std::any &val) -> int64_t {
			return ranges::min(generic(val));
		}};
	})
	.def_property_readonly("max", [](SequenceProxy<T> &seq) -> Proxy<int64_t> {
		auto generic = seq.getIteratorFunc();
		return std::function<int64_t(std::any &)>{[=](std::any &val) -> int64_t {
			return ranges::max(generic(val));
		}};
	})
	.def_property_readonly("sum", [](SequenceProxy<T> &seq) -> Proxy<int64_t> {
		auto generic = seq.getIteratorFunc();
		return std::function<int64_t(std::any &)>{[=](std::any &val) -> int64_t {
			return ranges::accumulate(generic(val), int64_t(0));
		}};
	})
	;
}

#endif /* proxy_arithmetic_range_hpp */