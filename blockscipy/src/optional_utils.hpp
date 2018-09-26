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

template<typename Proxy>
struct MakeOptionalFunctor {
	using ResultType = make_optional_t<typename Proxy::output_t>;
	Proxy p1;

	MakeOptionalFunctor(Proxy &p1_) : p1(p1_) {}

	ResultType operator()(const ranges::optional<typename Proxy::input_t> &val) const {
		if (val) {
			auto v = *val;
			return p1(v);
		} else {
			return ranges::nullopt;
		}
	}
};

#endif /* optional_utils_h */
