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
#include "method_tags.hpp"
#include "blocksci_type_converter.hpp"

#include <pybind11/pybind11.h>

template <typename P, typename Out, typename R> 
struct ApplyMethodsToProxyFuncBinder {
    using Func = std::function<R(Out &)>;
    Func func;

    ApplyMethodsToProxyFuncBinder(Func func_) : func(func_) {}

    auto operator()(Out && item) const -> decltype(BlockSciTypeConverter{}(func(item))) {
        return BlockSciTypeConverter{}(func(item));
    }
};

template <typename P, typename Out, typename R>
using proxy_apply_converter_t = ApplyMethodsToProxyFuncBinder<P, Out, R>;

template <typename P, typename Out, typename R, typename... Args>
struct ApplyMethodsToProxyFuncConverter {
    using Func = std::function<R(Out &, Args...)>;
    Func func;

    ApplyMethodsToProxyFuncConverter(Func func_ ) : func(func_) {}

    auto operator()(P &p, const Args & ...args) const -> decltype(lift(p, proxy_apply_converter_t<P, Out, R>{std::bind(func, std::placeholders::_1, args...)})) {
    	return lift(p, proxy_apply_converter_t<P, Out, R>{std::bind(func, std::placeholders::_1, args...)});
    }
};

template <typename P,  typename Out, typename R, typename... Args>
struct ApplyGenericMethodsToProxyFuncConverter {
    using Func = std::function<R(Out &, Args...)>;
    Func func;

    ApplyGenericMethodsToProxyFuncConverter(Func func_ ) : func(func_) {}

    auto operator()(P &p, const Args & ...args) const -> decltype(liftGeneric(p, proxy_apply_converter_t<P, Out, R>{std::bind(func, std::placeholders::_1, args...)})) {
        return liftGeneric(p, proxy_apply_converter_t<P, Out, R>{std::bind(func, std::placeholders::_1, args...)});
    }
};

template <template<typename, typename, typename, typename...> typename Converter, typename Out, typename Class>
struct ApplyMethodsToProxyImpl {
    using P = typename Class::type;
    Class &cl;

    ApplyMethodsToProxyImpl(Class &cl_) : cl(cl_) {}

    void applyPropertyImpl(const std::string &propertyName, const pybind11::cpp_function &func, const std::string &fullDescription) {
        cl.def_property_readonly(strdup(propertyName.c_str()), func, strdup(fullDescription.c_str()));
    }

    template <typename result_type>
    void applyProperty(const std::string &propertyName, std::function<result_type(Out &)> func, const std::string &description) {
        using converted_t = Converter<P, Out, result_type>;
        converted_t convertedFunc{func};
        applyPropertyImpl(propertyName, pybind11::cpp_function(std::move(convertedFunc), pybind11::return_value_policy::reference_internal), strdup(description.c_str()));
    }

    template <typename result_type, typename... Args, typename... Extra>
    void applyMethod(const std::string &propertyName, std::function<result_type(Out &, Args...)> func, const std::string &description, Extra && ...extra) {
        using converted_t = Converter<P, Out, result_type, Args...>;
        converted_t convertedFunc{func};
        cl.def(strdup(propertyName.c_str()), convertedFunc, std::forward<Extra>(extra)..., strdup(description.c_str()));
    }

    template <typename F, typename... Extra>
    void operator()(method_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        applyMethod(propertyName, func_adaptor<Out>(func), description, extra...);
    }

    template <typename F, typename... Extra>
    void operator()(property_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        applyProperty(propertyName, func_adaptor<Out>(func), description, extra...);
    }
};

template <typename Class, typename Applier>
void applyMethodsToProxy(Class &cl, Applier applier) {
    using P = typename Class::type;
    using Out = typename P::output_t;
    applier(ApplyMethodsToProxyImpl<ApplyMethodsToProxyFuncConverter, Out, Class>{cl});
}

template <typename Class, typename Applier>
void applyMethodsToProxyGeneric(Class &cl, Applier applier) {
    using P = typename Class::type;
    using Out = decltype(std::declval<P>().getGeneric()(std::declval<std::any &>()));
    applier(ApplyMethodsToProxyImpl<ApplyGenericMethodsToProxyFuncConverter, Out, Class>{cl});
}

#endif /* proxy_apply_py_hpp */