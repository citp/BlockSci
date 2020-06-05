//
//  range_map_optional_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_optional_impl_hpp
#define proxy_range_map_optional_impl_hpp

#include "proxy.hpp"
#include "proxy_py.hpp"
#include "proxy_utils.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace {
	constexpr auto isOptional = [](const auto &optional) { return static_cast<bool>(optional); };
	constexpr auto derefOptional = [](const auto &optional) { return *optional; };


	template <typename T>
	RawIterator<typename ranges::range_value_t<T>::value_type> flattenOptional(T && t) {
	    return ranges::views::transform(ranges::views::filter(std::forward<T>(t), isOptional), derefOptional);
	}
}

template<typename R>
Proxy<RawIterator<R>> mapOptional(IteratorProxy &p, Proxy<ranges::optional<R>> &p2) {
	return liftGeneric(p, [p2](auto && seq) -> RawIterator<R> {
		return flattenOptional(ranges::views::transform(std::forward<decltype(seq)>(seq).toAnySequence(), p2));
	});
}


#endif /* proxy_range_map_optional_impl_hpp */
