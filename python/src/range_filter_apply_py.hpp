//
//  range_filter_apply_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_filter_apply_py_h
#define range_filter_apply_py_h

#include "function_traits.hpp"
#include "method_types.hpp"
#include "range_conversion.hpp"
#include "blocksci_range.hpp"

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

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
        using result_type = typename Traits::result_type;
        
        if constexpr (ranges::Range<result_type>()) {
            return convertRangeToPython(std::invoke(func, t, args...));
        } else {
            return std::invoke(func, t, args...);
        }
        
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

template <template<typename> class Applier, typename T>
void applyRangeFiltersToRange(RangeClasses<T> &cls) {
    applyRangeMethodsToRange(cls.iterator, Applier<ranges::any_view<T>>{});
    applyRangeMethodsToRange(cls.range, Applier<ranges::any_view<T, ranges::category::random_access>>{});
}

#endif /* range_filter_apply_py_h */
