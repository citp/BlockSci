//
//  range_map_optional_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_optional_impl_hpp
#define proxy_range_map_optional_impl_hpp

#include "proxy_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace internal {
	constexpr auto isOptional = [](const auto &optional) { return static_cast<bool>(optional); };
	constexpr auto derefOptional = [](const auto &optional) { return *optional; };


	template <typename T>
	Iterator<typename ranges::range_value_t<T>::value_type> flattenOptional(T && t) {
	    return ranges::any_view<typename ranges::range_value_t<T>::value_type>{ranges::view::transform(ranges::view::filter(std::forward<T>(t), isOptional), derefOptional)};
	}
}

template<typename R>
Proxy<Iterator<R>> mapOptional(IteratorProxy &seq, Proxy<ranges::optional<R>> &p2) {
	auto generic = seq.getGeneric();
	return std::function<Iterator<R>(std::any &)>{[=](std::any &val) -> Iterator<R> {
		return internal::flattenOptional(ranges::view::transform(generic(val).rng, p2));
	}};
}


#endif /* proxy_range_map_optional_impl_hpp */
