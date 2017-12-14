//
//  variant_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/3/17.
//
//

#ifndef variant_py_h
#define variant_py_h

#include <pybind11/stl.h>

#include <mpark/variant.hpp>

namespace pybind11 { namespace detail {
    template <typename... Ts>
    struct type_caster<mpark::variant<Ts...>> : variant_caster<mpark::variant<Ts...>> {};
    
    // Specifies the function used to visit the variant -- `apply_visitor` instead of `visit`
    template <>
    struct visit_helper<mpark::variant> {
        template <typename... Args>
        static auto call(Args &&...args) -> decltype(mpark::visit(args...)) {
            return mpark::visit(args...);
        }
    };
}}

#endif /* variant_py_h */
