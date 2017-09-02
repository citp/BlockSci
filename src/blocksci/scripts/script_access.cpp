//
//  script_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_access.hpp"

#include "data_configuration.hpp"

namespace blocksci {
    ScriptAccess::ScriptAccess(const DataConfiguration &config) :
    scriptFiles(blocksci::apply(blocksci::ScriptInfoList(), [&] (auto tag) {
        return config.scriptsDirectory()/ blocksci::ScriptInfo<decltype(tag)::type>::typeName;
    })) {}
    
    
    template<ScriptType::Enum type>
    struct AddressCountFunctor {
        static size_t f(const ScriptAccess &access) {
            return access.addressCount<type>();
        }
    };
    
    size_t ScriptAccess::addressCount(ScriptType::Enum type) const {
        static constexpr auto table = make_dynamic_table<AddressCountFunctor>();
        static constexpr std::size_t size = ScriptType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](*this);
    }
    
    size_t ScriptAccess::totalAddressCount() const {
        uint32_t count = 0;
        for_each(scriptFiles, [&count](auto& obj) -> decltype(auto) {count += obj.size();});
        return count;
    }
}

