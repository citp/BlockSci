//
//  variant_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/3/17.
//
//

#ifndef variant_py_h
#define variant_py_h

#include <pybind11/pybind11.h>

#include <boost/variant/variant.hpp>

namespace pybind11 { namespace detail {
    template <typename... Ts>
    struct type_caster<boost::variant<Ts...>> : variant_caster<boost::variant<Ts...>> {};
    
    // Specifies the function used to visit the variant -- `apply_visitor` instead of `visit`
    template <>
    struct visit_helper<boost::variant> {
        template <typename... Args>
        static auto call(Args &&...args) -> decltype(boost::apply_visitor(args...)) {
            return boost::apply_visitor(args...);
        }
    };
}}

#endif /* variant_py_h */
