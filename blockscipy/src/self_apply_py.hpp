//
//  self_apply_py.hpp
//  blockscipy
//
//  Created by Malte Moeser on 8/23/19.
//

#ifndef self_apply_py_h
#define self_apply_py_h

#include "func_converter.hpp"
#include "method_tags.hpp"
#include "blocksci_type_converter.hpp"

#include <pybind11/pybind11.h>


template <typename Class>
struct ApplyMethodsToSelfImpl {
    using P = typename Class::type;
    Class &cl;

    ApplyMethodsToSelfImpl(Class &cl_) : cl(cl_) {}

    template <typename F, typename... Extra>
    void operator()(method_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        cl.def(strdup(propertyName.c_str()), func, std::forward<Extra>(extra)..., strdup(description.c_str()));
    }

    template <typename F, typename... Extra>
    void operator()(property_tag_type, const std::string &propertyName, F func, const std::string &description, Extra && ...extra) {
        cl.def_property_readonly(strdup(propertyName.c_str()), pybind11::cpp_function(std::move(func), pybind11::return_value_policy::reference_internal), strdup(description.c_str()));
    }
};

template <typename Class, typename Applier>
void applyMethodsToSelf(Class &cl, Applier applier) {
    applier(ApplyMethodsToSelfImpl<Class>{cl});
}

#endif /* self_apply_py_h */
