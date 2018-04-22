//
//  apply.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef blocksci_util_apply_h
#define blocksci_util_apply_h

#include <tuple>

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
    } // namespace internal
    
    template <class Tuple, class F>
    constexpr auto apply(Tuple && t, F f) {
        return internal::index_apply<std::tuple_size<std::decay_t<Tuple>>{}>([&](auto... Is) {
            return std::make_tuple(f(std::get<Is>(t))...);
        });
    }
    
    template<class Tuple, typename F>
    constexpr void for_each(Tuple&& t, F f) {
        blocksci::apply(std::forward<Tuple>(t), [&f](auto && item) {
            f(item);
            return 1;
        });
    }
} // namespace blocksci

#endif /* blocksci_util_apply_h */
