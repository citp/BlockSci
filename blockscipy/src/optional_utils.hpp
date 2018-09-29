//
//  optional_utils.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/25/18.
//

#ifndef optional_utils_h
#define optional_utils_h

#include <range/v3/utility/optional.hpp>

template <typename T>
struct remove_optional { using type = T; };

template <typename T>
struct remove_optional<ranges::optional<T>> { using type = T; };

template <typename T>
using remove_optional_t = typename remove_optional<T>::type;

// If type is already optional, do nothing. Otherwise make it optional
template <typename T>
struct make_optional { using type = ranges::optional<T>; };

template <typename T>
struct make_optional<ranges::optional<T>> { using type = ranges::optional<T>; };

template <typename T>
using make_optional_t = typename make_optional<T>::type;


#endif /* optional_utils_h */
