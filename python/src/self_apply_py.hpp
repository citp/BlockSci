//
//  self_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef self_py_h
#define self_py_h

#include "function_traits.hpp"
#include "range_conversion.hpp"

#include <blocksci/address/address_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <functional>

template <typename T>
struct SelfTypeConverter {
    auto operator()(const T &val) {
        return val;
    }
};

template<>
struct SelfTypeConverter<blocksci::Address> {
    blocksci::AnyScript operator()(const blocksci::Address &address);
};

template<>
struct SelfTypeConverter<uint16_t> {
    int64_t operator()(uint16_t val) {
        return static_cast<int64_t>(val);
    }
};

template<>
struct SelfTypeConverter<int16_t> {
    int64_t operator()(int16_t val) {
        return static_cast<int64_t>(val);
    }
};

template<>
struct SelfTypeConverter<int> {
    int64_t operator()(int val) {
        return static_cast<int64_t>(val);
    }
};

template<>
struct SelfTypeConverter<unsigned int> {
    int64_t operator()(int val) {
        return static_cast<int64_t>(val);
    }
};

// Potential overflow
template<>
struct SelfTypeConverter<uint64_t> {
    int64_t operator()(int val) {
        return static_cast<int64_t>(val);
    }
};

template <typename T, class Traits, typename F, typename S>
struct ApplyMethodToSelf;

template <typename T, class Traits, typename F, std::size_t ... Is>
struct ApplyMethodToSelf<T, Traits, F, std::index_sequence<Is...>> {
    F func;

    ApplyMethodToSelf(F func_) : func(std::move(func_)) {}

    auto operator()(T &t, const std::tuple_element_t<Is + 1, typename Traits::arg_tuple> &... args) const {
    	using result_type = decltype(std::invoke(func, t, args...));
    	if constexpr (!is_tagged<result_type>::value && ranges::Range<result_type>()) {
    		return convertRangeToPython(std::invoke(func, t, args...));
    	} else {
    		using Converter = SelfTypeConverter<decltype(std::invoke(func, t, args...))>;
    		return Converter{}(std::invoke(func, t, args...));
    	}
       
    }
};

template <typename T>
struct ApplyMethodsToSelfImpl {
    template <typename F>
    auto operator()(F func) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        return ApplyMethodToSelf<T, traits, F, arg_sequence>{func};
    }
};

template <typename Class, typename Applier>
auto applyMethodsToSelf(Class &cl, Applier applier) {
	applier(cl, ApplyMethodsToSelfImpl<typename Class::type>{});
}


#endif /* self_py_h */
