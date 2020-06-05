//
//  address_type_meta.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 2/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blocksci_address_type_meta_hpp
#define blocksci_address_type_meta_hpp

#include <blocksci/core/address_types.hpp>
#include <blocksci/core/meta.hpp>

#include <cstddef>
#include <functional>
#include <tuple>

namespace blocksci {
    template <template<AddressType::Enum> class K>
    using to_address_tuple_t = apply_template_t<AddressType::Enum, K, AddressType::all>;
}


#endif /* blocksci_address_type_meta_hpp */
