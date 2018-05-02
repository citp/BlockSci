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
#include "method_tags.hpp"

#include "range_conversion.hpp"
#include "blocksci_range.hpp"

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

enum class RangeMethodTypeTag {
    Range, Other
};

template <typename T>
constexpr RangeMethodTypeTag getRangeMethodTypeTag() {
    if constexpr (ranges::Range<T>()) {
        return RangeMethodTypeTag::Range;
    } else {
        return RangeMethodTypeTag::Other;
    }
}

template <typename result_type, RangeMethodTypeTag tag = getRangeMethodTypeTag<result_type>()>
struct RangeMethodConverter;

template <typename result_type>
struct RangeMethodConverter<result_type, RangeMethodTypeTag::Range> {
    using type = decltype(convertRangeToPython(std::declval<result_type>()));

    type operator()(result_type && result) const {
        return convertRangeToPython(result);
    }
};

template <typename result_type>
struct RangeMethodConverter<result_type, RangeMethodTypeTag::Other> {
    using type = result_type;

    type operator()(result_type && result) const {
        return result;
    }
};

template <typename Range, class Traits, typename F, typename S>
struct ApplyRangeMethodToRange;

template <typename Range, class Traits, typename F, std::size_t ... Is>
struct ApplyRangeMethodToRange<Range, Traits, F, std::index_sequence<Is...>> {
    using result_type = typename Traits::result_type;
    using converter = RangeMethodConverter<result_type>;
    using return_type = typename converter::type;
    F func;

    ApplyRangeMethodToRange(F func_) : func(std::move(func_)) {}

    return_type operator()(Range &t, const std::tuple_element_t<Is + 1, typename Traits::arg_tuple> &... args) const {
        return converter{}(std::invoke(func, t, args...));
    }
};


template <typename Class>
struct ApplyRangeMethodsToRangeImpl {
    using Range = typename Class::type;
    using value_type = ranges::range_value_type_t<Range>;

    Class &cl;

    ApplyRangeMethodsToRangeImpl(Class &cl_) : cl(cl_) {}

    template <typename F>
    void operator()(property_tag_type, const std::string &propertyName, F func, const std::string &description) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        using wrapped_func_type = ApplyRangeMethodToRange<Range, traits, F, arg_sequence>;
        
        PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<typename wrapped_func_type::return_type>::name();

        std::stringstream ss;
        ss << description << "\n\n:type: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";

        cl.def_property_readonly(strdup(propertyName.c_str()), wrapped_func_type{func}, strdup(ss.str().c_str()));
    }

    template <typename F, typename... Args>
    void operator()(method_tag_type, const std::string &propertyName, F func, const std::string &description, Args && ...args) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        using wrapped_func_type = ApplyRangeMethodToRange<Range, traits, F, arg_sequence>;
        
        cl.def(strdup(propertyName.c_str()), wrapped_func_type{func}, std::forward<Args>(args)..., strdup(description.c_str()));
    }
};

template <typename Class, typename Applier>
auto applyRangeMethodsToRange(Class &cl, Applier applier) {
    applier(ApplyRangeMethodsToRangeImpl{cl});
}

template <template<typename> class Applier, typename T>
void applyRangeFiltersToRange(RangeClasses<T> &cls) {
    applyRangeMethodsToRange(cls.iterator, Applier<ranges::any_view<T>>{});
    applyRangeMethodsToRange(cls.range, Applier<ranges::any_view<T, ranges::category::random_access>>{});
}

#endif /* range_filter_apply_py_h */
