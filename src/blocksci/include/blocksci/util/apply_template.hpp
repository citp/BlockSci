//
//  apply_template.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef apply_template_h
#define apply_template_h

#include <tuple>

namespace blocksci {
    
    namespace internal {
        template<typename Z, template<Z> class K, typename T>
        struct apply_template;
        
        template <typename Z, template<Z> class K, typename... Types>
        struct apply_template<Z, K, std::tuple<Types...>> {
            using type = std::tuple<K<Types::value>...>;
        };
    } // namespace internal
    
    template <typename T, template<T> class K, typename... Types>
    using apply_template_t = typename internal::apply_template<T, K, Types...>::type;
} // namespace blocksci

#endif /* apply_template_h */
