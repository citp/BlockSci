//
//  range_util.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//

#ifndef blocksci_range_util_hpp
#define blocksci_range_util_hpp

#include <blocksci/blocksci_export.h>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace blocksci {
    
    struct FlatMapOptionalsFilter {
        template <typename T>
        bool operator()(T && t) const {
            return static_cast<bool>(std::forward<T>(t));
        }
    };
    
    struct FlatMapOptionalsTransform {
        template <typename T>
        typename T::value_type operator()(T && t) const {
            return *std::forward<T>(t);
        }
    };
    
	inline auto BLOCKSCI_EXPORT flatMapOptionals() {
        return  ranges::views::filter(FlatMapOptionalsFilter{})
        | ranges::views::transform(FlatMapOptionalsTransform{});
    }
    
} // namespace blocksci

#endif // blocksci_range_util_hpp
