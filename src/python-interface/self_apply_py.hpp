//
//  self_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef self_py_h
#define self_py_h

#include "function_traits.hpp"

template <typename Ty, class F, std::size_t ... Is, class T>
auto applyMethodsToSelfImpl(F f, std::index_sequence<Is...>, T) {
    return [f](Ty &tx, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        return f(tx, args...);
    };
}

template <typename Ty,typename F>
auto applyMethodsToSelf(F f) {
    using traits = function_traits<F>;
    return applyMethodsToSelfImpl<Ty>(f, std::make_index_sequence<traits::arity>{}, traits{});
}

#endif /* self_py_h */
