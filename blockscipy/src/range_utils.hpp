//
//  range_utils.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef range_utils_hpp
#define range_utils_hpp

#include "proxy.hpp"

#include <range/v3/view/any_view.hpp>

constexpr ranges::category random_access_sized = ranges::category::random_access | ranges::category::sized;

template<typename T>
using Iterator = ranges::any_view<T>;

template<typename T>
using Range = ranges::any_view<T, ranges::category::random_access | ranges::category::sized>;


#endif /* range_utils_hpp */
