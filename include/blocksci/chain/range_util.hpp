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
	inline auto BLOCKSCI_EXPORT flatMapOptionals() {
	    return  ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
	    		| ranges::view::transform([](const auto &optional) { return *optional; });
    }
    
} // namespace blocksci

#endif // blocksci_range_util_hpp
