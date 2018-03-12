//
//  script_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_access.hpp"

namespace blocksci {
    ScriptAccess::ScriptAccess(const DataConfiguration &config_) :
    scriptFiles(blocksci::apply(DedupAddressInfoList(), [&] (auto tag) {
        return std::make_unique<ScriptFile<tag.value>>(config_.scriptsDirectory()/ std::string{dedupAddressName(tag)});
    })), config(config_) {}
    
    void ScriptAccess::reload() {
        for_each(scriptFiles, [&](auto& file) -> decltype(auto) { file->reload(); });
    }
    
    template<DedupAddressType::Enum type>
    struct ScriptCountFunctor {
        static uint32_t f(const ScriptAccess &access) {
            return static_cast<uint32_t>(access.scriptCount<type>());
        }
    };
    
    uint32_t ScriptAccess::scriptCount(DedupAddressType::Enum type) const {
        static constexpr auto table = make_dynamic_table<DedupAddressType, ScriptCountFunctor>();
        static constexpr std::size_t size = AddressType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](*this);
    }
    
    std::array<uint32_t, DedupAddressType::size> ScriptAccess::scriptCounts() const {
        return make_static_table<DedupAddressType, ScriptCountFunctor>(*this);
    }
    
    size_t ScriptAccess::totalAddressCount() const {
        uint32_t count = 0;
        for_each(scriptFiles, [&count](auto& obj) -> decltype(auto) {count += obj->size();});
        return count;
    }
}

