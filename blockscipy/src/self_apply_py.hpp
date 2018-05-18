//
//  self_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef self_py_h
#define self_py_h

#include "function_traits.hpp"
#include "func_converter.hpp"
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
    using return_type = result_type;
    return_type operator()(result_type && result) const {
        return std::move(result);
    }
};

template <typename result_type>
struct SelfApplyTypeConverterImpl<result_type, SelfApplyTag::Range> {
    using return_type = decltype(convertRangeToPython(std::declval<result_type>()));

    return_type operator()(result_type && result) const {
        return convertRangeToPython(result);
    }
};

template <typename result_type>
using self_apply_converter_t = SelfApplyTypeConverterImpl<result_type, getSelfApplyTag<result_type>()>;

template <typename Class>
struct ApplyMethodsToSelfImpl {
    using T = typename Class::type;

    Class &cl;

    ApplyMethodsToSelfImpl(Class &cl_) : cl(cl_) {}

    template <typename R, typename... Args, typename... Extra>
    void applyProperty(const std::string &propertyName, std::function<R(T &, Args...)> func, const std::string &description, Extra && ...extra) {
        using converter_type = self_apply_converter_t<R>;
        using return_type = typename converter_type::return_type;
        
        PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<return_type>::name();
        std::stringstream ss;
        ss << description << "\n\n:type: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";

        cl.def_property_readonly(strdup(propertyName.c_str()), [func](T &self, const Args &...args) {
            converter_type converter{};
            return converter(std::invoke(func, self, args...));
        }, std::forward<Extra>(extra)..., strdup(ss.str().c_str()));
    }

    template <typename R, typename... Args, typename... Extra>
    void applyProperty(const std::string &propertyName, std::function<R(const T &, Args...)> func, const std::string &description, Extra && ...extra) {
        using converter_type = self_apply_converter_t<R>;
        using return_type = typename converter_type::return_type;
        
        PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<return_type>::name();
        std::stringstream ss;
        ss << description << "\n\n:type: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";

        cl.def_property_readonly(strdup(propertyName.c_str()), [func](const T &self, const Args &...args) {
            converter_type converter{};
            return converter(std::invoke(func, self, args...));
        }, std::forward<Extra>(extra)..., strdup(ss.str().c_str()));
    }

    template <typename R, typename... Args, typename... Extra>
    void applyMethod(const std::string &propertyName, std::function<R(T &, Args...)> func, const std::string &description, Extra && ...extra) {
        using converter_type = self_apply_converter_t<R>;
        using return_type = typename converter_type::return_type;

        PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<return_type>::name();
        std::stringstream ss;
        ss << description << "\n\n:rtype: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";

        cl.def(strdup(propertyName.c_str()), [func](T &self, const Args &...args) {
            converter_type converter{};
            return converter(std::invoke(func, self, args...));
        }, std::forward<Extra>(extra)..., strdup(ss.str().c_str()));
    }

    template <typename R, typename... Args, typename... Extra>
    void applyMethod(const std::string &propertyName, std::function<R(const T &, Args...)> func, const std::string &description, Extra && ...extra) {
        using converter_type = self_apply_converter_t<R>;
        using return_type = typename converter_type::return_type;

        PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<return_type>::name();
        std::stringstream ss;
        ss << description << "\n\n:rtype: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";

        cl.def(strdup(propertyName.c_str()), [func](const T &self, const Args & ...args) {
            converter_type converter{};
            return converter(std::invoke(func, self, args...));
        }, std::forward<Extra>(extra)..., strdup(ss.str().c_str()));
    }

    template <typename F, typename... Extra>
    void operator()(method_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        applyMethod(propertyName, func_adaptor<T>(func), description, extra...);
    }

    template <typename F, typename... Extra>
    void operator()(property_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        applyProperty(propertyName, func_adaptor<T>(func), description, extra...);
    }
};

template <typename Class, typename Applier>
auto applyMethodsToSelf(Class &cl, Applier applier) {
	applier(ApplyMethodsToSelfImpl{cl});
}


#endif /* self_py_h */
