//
//  for_each.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef blocksci_util_for_each_h
#define blocksci_util_for_each_h

namespace blocksci {
    namespace internal {
        template<int I, class Tuple, typename F> struct for_each_impl {
            static constexpr void for_each(Tuple&& t, F f) {
                for_each_impl<I - 1, Tuple, F>::for_each(std::forward<Tuple>(t), f);
                f(std::get<I>(t));
            }
        };
        template<class Tuple, typename F> struct for_each_impl<0, Tuple, F> {
            static constexpr void for_each(Tuple&& t, F f) {
                f(std::get<0>(std::forward<Tuple>(t)));
            }
        };
    } // namespace internal
    
    template<class Tuple, typename F>
    constexpr void for_each(Tuple&& t, F f) {
        internal::for_each_impl<std::tuple_size<typename std::remove_reference<Tuple>::type>::value - 1, Tuple, F>::for_each(std::forward<Tuple>(t), f);
    }
} // namespace blocksci

#endif /* blocksci_util_for_each_h */
