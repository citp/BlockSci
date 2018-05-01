//
//  self_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef self_py_h
#define self_py_h

#include "type_converter.hpp"
#include "function_traits.hpp"
#include "range_conversion.hpp"
#include "method_types.hpp"

#include <blocksci/address/address_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <functional>

enum class SelfApplyTag {
    Range, Normal
};

template <typename T>
constexpr SelfApplyTag getSelfApplyTag() {
    if constexpr (isNonTaggedRange<T>()) {
        return SelfApplyTag::Range;
    } else {
        return SelfApplyTag::Normal;
    }
}

template <typename result_type, SelfApplyTag tag>
struct SelfApplyTypeConverterImpl;

template <typename result_type>
struct SelfApplyTypeConverterImpl<result_type, SelfApplyTag::Normal> {
    using return_type = decltype(BasicTypeConverter{}(std::declval<result_type>()));
    static_assert(std::is_same_v<decltype(PythonTypeName<return_type>::name()), std::string>);

    return_type operator()(result_type && result) const {
        return BasicTypeConverter{}(result);
    }
};

template <typename result_type>
struct SelfApplyTypeConverterImpl<result_type, SelfApplyTag::Range> {
    using return_type = decltype(convertRangeToPython(std::declval<result_type>()));
    static_assert(std::is_same_v<decltype(PythonTypeName<return_type>::name()), std::string>);

    return_type operator()(result_type && result) const {
        return convertRangeToPython(result);
    }
};

template <typename result_type>
using self_apply_converter_t = SelfApplyTypeConverterImpl<result_type, getSelfApplyTag<result_type>()>;

template <typename T, class Traits, typename F, typename S>
struct ApplyMethodToSelf;

template <typename T, class Traits, typename F, std::size_t ... Is>
struct ApplyMethodToSelf<T, Traits, F, std::index_sequence<Is...>> {
    using converter_type = self_apply_converter_t<typename Traits::result_type>;
    using return_type = typename converter_type::return_type;
    static_assert(std::is_same_v<decltype(PythonTypeName<return_type>::name()), std::string>);

    F func;
    converter_type converter{};

    ApplyMethodToSelf(F func_) : func(std::move(func_)) {}
    return_type operator()(T &t, const std::tuple_element_t<Is + 1, typename Traits::arg_tuple> &... args) const {
        return converter(std::invoke(func, t, args...));
    }
};

template <typename Class>
struct ApplyMethodsToSelfImpl {
    using T = typename Class::type;

    Class &cl;

    ApplyMethodsToSelfImpl(Class &cl_) : cl(cl_) {}

    template <typename F, typename... Args>
    void operator()(property_tag_type, const std::string &propertyName, F func, const std::string &description, Args && ...args) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        using wrapped_func_type = ApplyMethodToSelf<T, traits, F, arg_sequence>;
        
        PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<typename wrapped_func_type::return_type>::name();

        std::stringstream ss;
        ss << description << "\n\n:type: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";

        cl.def_property_readonly(strdup(propertyName.c_str()), wrapped_func_type{func}, std::forward<Args>(args)..., strdup(ss.str().c_str()));
    }

    template <typename F, typename... Args>
    void operator()(method_tag_type, const std::string &propertyName, F func, const std::string &description, Args && ...args) {
        using traits = function_traits<F>;
        using arg_sequence = std::make_index_sequence<traits::arity - 1>;
        using wrapped_func_type = ApplyMethodToSelf<T, traits, F, arg_sequence>;
        std::stringstream ss;
        ss << description;

        cl.def(strdup(propertyName.c_str()), wrapped_func_type{func}, std::forward<Args>(args)..., strdup(ss.str().c_str()));
    }
};

template <typename Class, typename Applier>
auto applyMethodsToSelf(Class &cl, Applier applier) {
	applier(ApplyMethodsToSelfImpl{cl});
}


#endif /* self_py_h */
