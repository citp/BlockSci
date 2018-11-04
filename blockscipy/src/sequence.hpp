//
//  sequence.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/4/18.
//

#ifndef blocksci_sequence_hpp
#define blocksci_sequence_hpp

#include <range/v3/view/any_view.hpp>

constexpr ranges::category random_access_sized = ranges::category::random_access | ranges::category::sized;

template <typename T>
using RawIterator = ranges::any_view<T>;

template <typename T>
using RawRange = ranges::any_view<T, random_access_sized>;


#endif /* blocksci_sequence_hpp */
