//
//  range_apply_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef range_apply_py_h
#define range_apply_py_h

#include "func_converter.hpp"
#include "method_types.hpp"
#include "method_tags.hpp"
#include "range_conversion.hpp"
#include "blocksci_range.hpp"
#include "blocksci_type_converter.hpp"

#include <range/v3/view/transform.hpp>

template <typename T, typename R> 
struct ApplyMethodsToRangeFuncBinder {
    using Func = std::function<R(T &)>;
    Func func;

    ApplyMethodsToRangeFuncBinder(Func func_) : func(func_) {}

    auto operator()(T && item) const -> decltype(BlockSciTypeConverter{}(func(item))) {
        return BlockSciTypeConverter{}(func(item));
    }
};

template <typename Range, typename R, typename... Args>
struct ApplyMethodsToRangeFuncConverter {
    using T = ranges::range_value_type_t<Range>;
    using Func = std::function<R(T &, Args...)>;
    using return_type = decltype(convertRangeToPython(ranges::view::transform(std::declval<Range &>(), ApplyMethodsToRangeFuncBinder<T, R>{std::bind(std::declval<Func>(), std::placeholders::_1, std::declval<const Args &>()...)})));
    Func func;

    ApplyMethodsToRangeFuncConverter(Func func_ ) : func(func_) {}

    return_type operator()(Range &t, const Args & ...args) const {
        return convertRangeToPython(ranges::view::transform(t, ApplyMethodsToRangeFuncBinder<T, R>{std::bind(func, std::placeholders::_1, args...)}));
    }
};

template <typename T>
std::string methodDescription(const std::string description) {
    PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<T>::name();
    std::stringstream ss;
    ss << "For each item: " << description << "\n\n:rtype: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";
    return ss.str();
}

template <typename T>
std::string propertyDescription(const std::string description) {
    PYBIND11_DESCR returnTypeDescr = pybind11::detail::make_caster<T>::name();
    std::stringstream ss;
    ss << "For each item: " << description << "\n\n:type: :class:`" << getTypeName(returnTypeDescr.text(), returnTypeDescr.types()) << "`";
    return ss.str();
}

template <typename Class>
struct ApplyMethodsToRangeImpl {
    using Range = typename Class::type;
    using value_type = ranges::range_value_type_t<Range>;
    Class &cl;

    ApplyMethodsToRangeImpl(Class &cl_) : cl(cl_) {}

    void applyPropertyImpl(const std::string &propertyName, const pybind11::cpp_function &func, const std::string &fullDescription) {
        cl.def_property_readonly(strdup(propertyName.c_str()), func, strdup(fullDescription.c_str()));
    }

    template <typename result_type>
    void applyProperty(const std::string &propertyName, std::function<result_type(value_type &)> func, const std::string &description) {
        using converted_t = ApplyMethodsToRangeFuncConverter<Range, result_type>;
        converted_t convertedFunc{func};
        std::string fullDescription = propertyDescription<typename converted_t::return_type>(description);
        applyPropertyImpl(propertyName, pybind11::cpp_function(std::move(convertedFunc), pybind11::return_value_policy::reference_internal), fullDescription);
    }

    template <typename result_type, typename... Args, typename... Extra>
    void applyMethod(const std::string &propertyName, std::function<result_type(value_type &, Args...)> func, const std::string &description, Extra && ...extra) {
        using converted_t = ApplyMethodsToRangeFuncConverter<Range, result_type, Args...>;
        converted_t convertedFunc{func};
        std::string fullDescription = methodDescription<typename converted_t::return_type>(description);
        cl.def(strdup(propertyName.c_str()), convertedFunc, std::forward<Extra>(extra)..., strdup(fullDescription.c_str()));
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
void applyMethodsToRange(Class &cl, Applier applier) {
    applier(ApplyMethodsToRangeImpl{cl});
}

template <typename T, typename Applier>
void applyAllMethodsToRange(T &cls, Applier applier) {
    applyMethodsToRange(cls.iterator, applier);
    applyMethodsToRange(cls.range, applier);
}

#endif /* range_apply_py_h */
