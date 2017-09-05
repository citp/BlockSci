//
//  optional_py.h
//  blocksci
//
//  Created by Harry Kalodner on 9/3/17.
//
//

#ifndef optional_py_h
#define optional_py_h

#include <pybind11/stl.h>
#include <boost/optional/optional.hpp>

namespace pybind11 { namespace detail {
    template <typename T>
    struct type_caster<boost::optional<T>> : optional_caster<boost::optional<T>> {};
}}

#endif /* optional_py_h */
