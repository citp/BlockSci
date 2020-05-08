//
//  range_map_sequence_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_sequence_impl_hpp
#define proxy_range_map_sequence_impl_hpp

#include "proxy.hpp"
#include "proxy_utils.hpp"

#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

template<typename R>
Proxy<RawIterator<R>> mapSequence(IteratorProxy &p, Proxy<RawIterator<R>> &p2) {
	return liftGeneric(p, [p2](auto && seq) -> RawIterator<R> {
		return ranges::views::join(ranges::views::transform(std::forward<decltype(seq)>(seq).toAnySequence(), p2));
	});
}

#endif /* proxy_range_map_optional_hpp */
