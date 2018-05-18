//
//  script_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#include <blocksci/scripts/script_range.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>

namespace {
    template<blocksci::AddressType::Enum type>
    struct ScriptRangeFunctor {
        static blocksci::ScriptRangeVariant f(blocksci::DataAccess &access) {
            return blocksci::scriptsRange<type>(access);
        }
    };
}
    

namespace blocksci {

    uint32_t getScriptCount(AddressType::Enum type, blocksci::DataAccess &access) {
        return access.getScripts().scriptCount(dedupType(type));
    }
    

    ScriptRangeVariant scriptsRange(AddressType::Enum type, blocksci::DataAccess &access) {
        static constexpr auto table = make_dynamic_table<AddressType, ScriptRangeFunctor>();
        auto index = static_cast<size_t>(type);
        return table.at(index)(access);
    }
   
} // namespace blocksci
