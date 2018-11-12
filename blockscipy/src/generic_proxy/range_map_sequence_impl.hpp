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
Proxy<RawIterator<R>> mapSequence(IteratorProxy &seq, Proxy<RawIterator<R>> &p2) {
	return std::function<RawIterator<R>(std::any &)>{[generic = seq.getGenericIterator(), p2](std::any &val) -> RawIterator<R> {
		return ranges::view::join(ranges::view::transform(generic(val), [p2](BlocksciType && v) -> RawIterator<R> {
			return p2.applySimple(v.toAny());
		}));
	}};
}

#endif /* proxy_range_map_optional_hpp */
