//
//  script_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 5/8/18.
//

#ifndef script_range_h
#define script_range_h

#include <blocksci/blocksci_export.h>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

// 

namespace blocksci {
    
    template <AddressType::Enum type>
    using ScriptRange = ranges::any_view<ScriptAddress<type>, ranges::category::random_access | ranges::category::sized>;
    using ScriptRangeVariant = to_variadic_t<to_address_tuple_t<ScriptRange>, mpark::variant>;
    
    uint32_t BLOCKSCI_EXPORT getScriptCount(AddressType::Enum type, blocksci::DataAccess &access);

    template <AddressType::Enum type>
    ScriptRange<type> BLOCKSCI_EXPORT scriptsRange(DataAccess &access) {
        auto scriptCount = getScriptCount(type, access);
        return ranges::view::ints(uint32_t{1}, scriptCount + 1) | ranges::view::transform([&](uint32_t scriptNum) {
            return ScriptAddress<type>(scriptNum, access);
        });
    }
    
    ScriptRangeVariant BLOCKSCI_EXPORT scriptsRange(AddressType::Enum type, DataAccess &access);

    
}

#endif /* script_range_h */
