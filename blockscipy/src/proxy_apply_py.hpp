//
//  proxy_apply_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_apply_py_hpp
#define proxy_apply_py_hpp

#include "proxy_utils.hpp"
#include "func_converter.hpp"
#include "self_apply_py.hpp"
#include "method_tags.hpp"
#include "blocksci_type_converter.hpp"

#include <pybind11/pybind11.h>

template <typename P, typename R, SelfApplyTag tag> 
struct ApplyMethodsToProxyFuncBinder;

template <typename P, typename R> 
struct ApplyMethodsToProxyFuncBinder<P, R, SelfApplyTag::Normal> {
	using output_t = typename P::output_t;
    using Func = std::function<R(output_t &)>;
    Func func;

    ApplyMethodsToProxyFuncBinder(Func func_) : func(func_) {}

    auto operator()(output_t && item) const -> decltype(BlockSciTypeConverter{}(func(item))) {
        return BlockSciTypeConverter{}(func(item));
    }
};

template <typename P, typename R> 
struct ApplyMethodsToProxyFuncBinder<P, R, SelfApplyTag::Range> {
    using output_t = typename P::output_t;
    using Func = std::function<R(output_t &)>;
    Func func;

    ApplyMethodsToProxyFuncBinder(Func func_) : func(func_) {}

    auto operator()(output_t && item) const -> decltype(convertRangeToPython(func(item))) {
        return convertRangeToPython(func(item));
    }
};

template <typename P, typename R>
using proxy_apply_converter_t = ApplyMethodsToProxyFuncBinder<P, R, getSelfApplyTag<R>()>;


template <typename P, typename R, typename... Args>
struct ApplyMethodsToProxyFuncConverter {
    using Func = std::function<R(typename P::output_t &, Args...)>;
    Func func;

    ApplyMethodsToProxyFuncConverter(Func func_ ) : func(func_) {}

    auto operator()(P &p, const Args & ...args) const -> decltype(lift(p, proxy_apply_converter_t<P, R>{std::bind(func, std::placeholders::_1, args...)})) {
    	return lift(p, proxy_apply_converter_t<P, R>{std::bind(func, std::placeholders::_1, args...)});
    }
};

// template <typename T>
// std::string methodDescription(const std::string description) {
//     PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<T>::name();
//     std::stringstream ss;
//     ss << "For each item: " << description << "\n\n:rtype: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";
//     return ss.str();
// }

// template <typename T>
// std::string propertyDescription(const std::string description) {
//     PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<T>::name();
//     std::stringstream ss;
//     ss << "For each item: " << description << "\n\n:type: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";
//     return ss.str();
// }

template <typename Class>
struct ApplyMethodsToProxyImpl {
    using P = typename Class::type;
    using value_type = typename P::output_t;
    Class &cl;

    ApplyMethodsToProxyImpl(Class &cl_) : cl(cl_) {}

    void applyPropertyImpl(const std::string &propertyName, const pybind11::cpp_function &func, const std::string &fullDescription) {
        cl.def_property_readonly(strdup(propertyName.c_str()), func, strdup(fullDescription.c_str()));
    }

    template <typename result_type>
    void applyProperty(const std::string &propertyName, std::function<result_type(value_type &)> func, const std::string &description) {
        using converted_t = ApplyMethodsToProxyFuncConverter<P, result_type>;
        converted_t convertedFunc{func};
        // std::string fullDescription = propertyDescription<typename converted_t::return_type>(description);
        applyPropertyImpl(propertyName, pybind11::cpp_function(std::move(convertedFunc), pybind11::return_value_policy::reference_internal), strdup(description.c_str()));
    }

    template <typename result_type, typename... Args, typename... Extra>
    void applyMethod(const std::string &propertyName, std::function<result_type(value_type &, Args...)> func, const std::string &description, Extra && ...extra) {
        using converted_t = ApplyMethodsToProxyFuncConverter<P, result_type, Args...>;
        converted_t convertedFunc{func};
        // std::string fullDescription = methodDescription<typename converted_t::return_type>(description);
        cl.def(strdup(propertyName.c_str()), convertedFunc, std::forward<Extra>(extra)..., strdup(description.c_str()));
    }

    template <typename F, typename... Extra>
    void operator()(method_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        applyMethod(propertyName, func_adaptor<value_type>(func), description, extra...);
    }

    template <typename F, typename... Extra>
    void operator()(property_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        applyProperty(propertyName, func_adaptor<value_type>(func), description, extra...);
    }
};

template <typename Class, typename Applier>
void applyMethodsToProxy(Class &cl, Applier applier) {
    applier(ApplyMethodsToProxyImpl{cl});
}

#endif /* proxy_apply_py_hpp */