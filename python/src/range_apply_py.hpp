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

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<ranges::optional<T>> : std::true_type {};

// If type is already optional, do nothing. Otherwise make it optional
template <typename T>
struct make_optional { using type = ranges::optional<T>; };

template <typename T>
struct make_optional<ranges::optional<T>> { using type = ranges::optional<T>; };

template <typename T>
using make_optional_t = typename make_optional<T>::type;

template <typename T, class F, std::size_t ... Is, class Trait, typename... Args>
auto applyMethodToRangeImpl(T && t, F func, std::index_sequence<Is...>, Trait, std::true_type, const Args &... args) {
    return std::forward<T>(t) | ranges::view::transform([=](auto && item) -> make_optional_t<decltype(func(*item, args...))> {
        if (item) {
            return func(*item, args...);
        } else {
            return ranges::nullopt;
        }
    });
}

template <typename T, class F, std::size_t ... Is, class Trait, typename... Args>
auto applyMethodToRangeImpl(T && t, F func, std::index_sequence<Is...>, Trait, std::false_type, const Args &... args) {
    return std::forward<T>(t) | ranges::view::transform([=](auto && item) {
        return func(item, args...);
    });
}

template <typename T, typename F, typename... Args, CONCEPT_REQUIRES_(ranges::Range<T>())>
auto applyMethodToRange(T && t, F func, const Args &... args) {
    using traits = function_traits<F>;
    return applyMethodToRangeImpl<T>(std::forward<T>(t), func, std::make_index_sequence<traits::arity>{}, traits{}, is_optional<ranges::range_value_type_t<T>>{}, args...);
}

template <typename T>
auto flattenIfOptionalRangeImpl(T && t, std::true_type) {
    return std::forward<T>(t) | ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
    | ranges::view::transform([](const auto &optional) { return *optional; });
}

template <typename T>
auto flattenIfOptionalRangeImpl(T && t, std::false_type) {
    return std::forward<T>(t);
}

template <typename T>
auto flattenIfOptionalRange(T && t) {
    return flattenIfOptionalRangeImpl<T>(std::forward<T>(t), is_optional<ranges::range_value_type_t<T>>{});
}

template <typename T, typename F, std::size_t ... Is, class Traits>
auto applyMethodsToRangeImpl(F f, std::index_sequence<Is...>, Traits) {
    return [f](T &view, const std::tuple_element_t<Is, typename Traits::arg_tuple> &... args) {
        return convertRangeToPython(applyMethodToRange(view, f, args...));
    };
}

template <typename T, typename F>
auto applyMethodsToRange(F f) {
    using traits = function_traits<F>;
    return applyMethodsToRangeImpl<T>(f, std::make_index_sequence<traits::arity>{}, traits{});
}

template <typename T, typename F, std::size_t ... Is, class Traits>
auto applyRangeMethodsToRangeImpl(F func, std::index_sequence<Is...>, Traits) {
    return [func](T &view, const std::tuple_element_t<Is, typename Traits::arg_tuple> &... args) {
        return convertRangeToPython(func(flattenIfOptionalRange(view), args...));
    };
}

template <typename T, typename F>
auto applyRangeMethodsToRangeImpl1(F func) {
    using traits = function_traits<F>;
    return applyRangeMethodsToRangeImpl<T>(func, std::make_index_sequence<traits::arity>{}, traits{});
}

template <template <typename, typename, typename> class F, typename Class>
auto applyRangeMethodsToRange(Class &cl) {
    using Range = typename Class::type;
    using flattened_type = decltype(flattenIfOptionalRange(std::declval<Range>()));
    auto handler = [](auto func) {
        return applyRangeMethodsToRangeImpl1<flattened_type>(func);
    };
    F<flattened_type, decltype(cl), decltype(handler)>{}(cl, handler);
}

#endif /* range_apply_py_h */
