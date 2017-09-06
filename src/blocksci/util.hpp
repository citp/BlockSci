//
//  util.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/6/17.
//
//

#ifndef util_h
#define util_h

#include <utility>

namespace blocksci {
    namespace internal {
        template <class F, size_t... Is>
        constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
            return f(std::integral_constant<size_t, Is> {}...);
        }
        
        template <size_t N, class F>
        constexpr auto index_apply(F f) {
            return index_apply_impl(f, std::make_index_sequence<N>{});
        }
    }
}

#endif /* util_h */
