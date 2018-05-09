//
//  script_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 5/8/18.
//

#ifndef script_range_h
#define script_range_h

namespace blocksci {
    
    template <AddressType::Enum type>
    using ScriptRange = ranges::any_view<ScriptAddress<type>, ranges::category::random_access | ranges::category::sized>;
    using ScriptRangeVariant = to_variadic_t<to_address_tuple_t<ScriptRange>, mpark::variant>;
    
    namespace internal {
        template<blocksci::AddressType::Enum type>
        struct ScriptRangeFunctor {
            static blocksci::ScriptRangeVariant f(blocksci::Blockchain &chain) {
                return chain.scripts<type>();
            }
        };
    }
    
    ScriptRangeVariant Blockchain::scripts(AddressType::Enum type) {
        static constexpr auto table = make_dynamic_table<AddressType, ScriptRangeFunctor>();
        auto index = static_cast<size_t>(type);
        return table.at(index)(*this);
    }
    
    template <AddressType::Enum type>
    ScriptRange<type> scripts(DataAccess &access) {
        return ranges::view::ints(uint32_t{1}, access.getScripts().scriptCount(dedupType(type)) + 1) | ranges::view::transform([&](uint32_t scriptNum) {
            return ScriptAddress<type>(scriptNum, access);
        });
    }
    
    ScriptRangeVariant scripts(AddressType::Enum type, DataAccess &access);
}

#endif /* script_range_h */
