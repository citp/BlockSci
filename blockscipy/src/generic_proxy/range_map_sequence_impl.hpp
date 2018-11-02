//
//  range_map_sequence_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_sequence_impl_hpp
#define proxy_range_map_sequence_impl_hpp

#include "proxy.hpp"

#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

template<typename R>
Proxy<Iterator<R>> mapSequence(IteratorProxy &seq, Proxy<Iterator<R>> &p2) {
	auto generic = seq.getGeneric();
	return std::function<Iterator<R>(std::any &)>{[=](std::any &val) -> Iterator<R> {
		return ranges::any_view<R>{ranges::view::join(ranges::view::transform(generic(val).rng, 
			[p2](std::any && v) -> ranges::any_view<R> {
				return p2(v).rng;
			}
		))};
	}};
}

#endif /* proxy_range_map_optional_hpp */
