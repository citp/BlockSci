//
//  range_apply_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_apply_py_h
#define range_apply_py_h

#include "function_traits.hpp"
#include "method_types.hpp"
#include "method_tags.hpp"
#include "range_conversion.hpp"
#include "blocksci_range.hpp"

template <typename T>
struct remove_optional { using type = T; };

template <typename T>
struct remove_optional<ranges::optional<T>> { using type = T; };

template <typename T>
using remove_optional_t = typename remove_optional<T>::type;

enum class WrappedRangeTypeTag {
    Normal, Optional
};

template <typename T>
constexpr WrappedRangeTypeTag getWrappedRangeTypeTag() {
    if constexpr (is_optional<ranges::range_value_type_t<T>>::value) {
        return WrappedRangeTypeTag::Optional;
    } else {
        return WrappedRangeTypeTag::Normal;
    }
}

template <ranges::category range_cat, typename result_type, WrappedRangeTypeTag tag>
struct WrappedRangeMethodTagImpl;

template <ranges::category range_cat, typename result_type>
struct WrappedRangeMethodTagImpl<range_cat, result_type, WrappedRangeTypeTag::Normal> {
    using type = decltype(convertRangeToPython(std::declval<ranges::any_view<result_type, range_cat>>()));
};

template <ranges::category range_cat, typename result_type>
struct WrappedRangeMethodTagImpl<range_cat, result_type, WrappedRangeTypeTag::Optional> {
    using type = decltype(convertRangeToPython(std::declval<ranges::any_view<make_optional_t<result_type>, range_cat>>()));
};

template <typename Range, typename result_type>
using wrapped_range_method_t = typename WrappedRangeMethodTagImpl<getBlockSciCategory(ranges::get_categories<Range>()), result_type, getWrappedRangeTypeTag<Range>()>::type;

template <typename Range, class Traits, typename F, typename S>
struct ApplyMethodToRange;

template <typename Range, class Traits, typename F, std::size_t ... Is>
struct ApplyMethodToRange<Range, Traits, F, std::index_sequence<Is...>> {
    using result_type = typename Traits::result_type;
    using return_type = wrapped_range_method_t<Range, result_type>;
    F func;

    ApplyMethodToRange(F func_) : func(std::move(func_)) {}

    wrapped_range_method_t<Range, result_type> operator()(Range &t, const std::tuple_element_t<Is + 1, typename Traits::arg_tuple> &... args) const {
        if constexpr (is_optional<ranges::range_value_type_t<Range>>{}) {
            using optional_ret = make_optional_t<result_type>;
            return convertRangeToPython(t | ranges::view::transform([&](auto && item) -> optional_ret {
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

template <typename Range, typename result_type>
struct ApplyPropertyToRange {
    using value_type = ranges::range_value_type_t<Range>;
    using return_type = wrapped_range_method_t<Range, result_type>;
    using FuncType = std::function<result_type(remove_optional_t<value_type> &)>;

    FuncType func;

    ApplyPropertyToRange(FuncType func_) : func(std::move(func_)) {}

    wrapped_range_method_t<Range, result_type> operator()(Range &t) const {
        if constexpr (is_optional<value_type>{}) {
            using optional_ret = make_optional_t<result_type>;
            return convertRangeToPython(t | ranges::view::transform([&](auto && item) -> optional_ret {
                if (item) {
                    return std::invoke(func, *item);
                } else {
                    return ranges::nullopt;
                }
            }));
        } else {
            return convertRangeToPython(t | ranges::view::transform([&](auto && item) {
                return std::invoke(func, item);
            }));
        }
    }
};

template <typename Class>
struct ApplyMethodsToRangeImpl {
    using Range = typename Class::type;
    using value_type = ranges::range_value_type_t<Range>;

    Class &cl;

    ApplyMethodsToRangeImpl(Class &cl_) : cl(cl_) {}

    template <typename F>
    void operator()(property_tag_type, const std::string &propertyName, F func, const std::string &description) {
        using traits = function_traits<F>;
        using result_type = typename traits::result_type;
        using wrapped_func_type = ApplyPropertyToRange<Range, result_type>;
        
        PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<typename wrapped_func_type::return_type>::name();

        std::stringstream ss;
        ss << "For each item " << description << "\n\n:type: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";

        cl.def_property_readonly(strdup(propertyName.c_str()), wrapped_func_type{func}, strdup(ss.str().c_str()));
    }

    template <typename F, typename... Args>
    void operator()(method_tag_type, const std::string &propertyName, F func, const std::string &description, Args && ...args) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        using wrapped_func_type = ApplyMethodToRange<Range, traits, F, arg_sequence>;
        
        std::stringstream ss;
        ss <<"For each item " << description;

        cl.def(strdup(propertyName.c_str()), wrapped_func_type{func}, std::forward<Args>(args)..., strdup(ss.str().c_str()));
    }
};

template <typename Class, typename Applier>
void applyMethodsToRange(Class &cl, Applier applier) {
    applier(ApplyMethodsToRangeImpl{cl});
}

template <typename T, typename Applier>
void applyMethodsToRange(RangeClasses<T> &cls, Applier applier) {
    applyMethodsToRange(cls.iterator, applier);
    applyMethodsToRange(cls.range, applier);
    applyMethodsToRange(cls.optionalIterator, applier);
    applyMethodsToRange(cls.optionalRange, applier);
}

#endif /* range_apply_py_h */
