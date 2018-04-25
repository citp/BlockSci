//
//  range_apply_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_apply_py_h
#define range_apply_py_h

#include "function_traits.hpp"
#include "range_conversion.hpp"

#include <range/v3/view/filter.hpp>

// If type is already optional, do nothing. Otherwise make it optional
template <typename T>
struct make_optional { using type = ranges::optional<T>; };

template <typename T>
struct make_optional<ranges::optional<T>> { using type = ranges::optional<T>; };

template <typename T>
using make_optional_t = typename make_optional<T>::type;


template <typename Range, class Traits, typename F, typename S>
struct ApplyMethodToRange;

template <typename Range, class Traits, typename F, std::size_t ... Is>
struct ApplyMethodToRange<Range, Traits, F, std::index_sequence<Is...>> {
    F func;

    ApplyMethodToRange(F func_) : func(std::move(func_)) {}

    auto operator()(Range &t, const std::tuple_element_t<Is + 1, typename Traits::arg_tuple> &... args) const {
        if constexpr (is_optional<ranges::range_value_type_t<Range>>{}) {
            return convertRangeToPython(t | ranges::view::transform([&](auto && item) -> make_optional_t<decltype(std::invoke(func, *item, args...))> {
                if (item) {
                    return std::invoke(func, *item, args...);
                } else {
                    return ranges::nullopt;
                }
            }));
        } else {
            return convertRangeToPython(t | ranges::view::transform([&](auto && item) {
                return std::invoke(func, item, args...);
            }));
        }
    }
};

template <typename T>
struct ApplyMethodsToRangeImpl {
    template <typename F>
    auto operator()(F func) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        return ApplyMethodToRange<T, traits, F, arg_sequence>{func};
    }
};

template <typename Class, typename Applier>
auto applyMethodsToRange(Class &cl, Applier applier) {
    applier(cl, ApplyMethodsToRangeImpl<typename Class::type>{});
}


template <typename T>
auto flattenIfOptionalRange(T && t) {
    if constexpr (is_optional<ranges::range_value_type_t<T>>{}) {
        return std::forward<T>(t) 
        | ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
        | ranges::view::transform([](const auto &optional) { return *optional; });
    } else {
        return std::forward<T>(t);
    }
}

template <typename Range, class Traits, typename F, typename S>
struct ApplyRangeMethodToRange;

template <typename Range, class Traits, typename F, std::size_t ... Is>
struct ApplyRangeMethodToRange<Range, Traits, F, std::index_sequence<Is...>> {
    F func;

    ApplyRangeMethodToRange(F func_) : func(std::move(func_)) {}

    auto operator()(Range &t, const std::tuple_element_t<Is + 1, typename Traits::arg_tuple> &... args) const {
        return convertRangeToPython(std::invoke(func, t, args...));
    }
};

template <typename T>
struct ApplyRangeMethodsToRangeImpl {
    template <typename F>
    auto operator()(F func) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        return ApplyRangeMethodToRange<T, traits, F, arg_sequence>{func};
    }
};

template <typename Class, typename Applier>
auto applyRangeMethodsToRange(Class &cl, Applier applier) {
    applier(cl, ApplyRangeMethodsToRangeImpl<typename Class::type>{});
}

#endif /* range_apply_py_h */
