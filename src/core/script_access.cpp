//
//  script_access.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include <blocksci/core/script_access.hpp>
#include <blocksci/core/dedup_address_info.hpp>

#include <boost/filesystem/path.hpp>

namespace blocksci {
    
    namespace internal {
        template<DedupAddressType::Enum type>
        struct ScriptCountFunctor {
            static uint32_t f(const ScriptAccess &access) {
                return static_cast<uint32_t>(access.getFile<type>().size());
            }
        };
    } // namespace internal
    
    ScriptAccess::ScriptAccess(const std::string &baseDirectory) :
        scriptFiles(blocksci::apply(DedupAddressType::all(), [&] (auto tag) {
            return std::make_unique<ScriptFile<tag.value>>((boost::filesystem::path{baseDirectory}/std::string{dedupAddressName(tag)}).native());
        })) {}
    
    std::array<uint32_t, DedupAddressType::size> ScriptAccess::scriptCounts() const {
        return make_static_table<DedupAddressType, internal::ScriptCountFunctor>(*this);
    }
    
    uint32_t ScriptAccess::scriptCount(DedupAddressType::Enum type) const {
        static constexpr auto table = make_dynamic_table<DedupAddressType, internal::ScriptCountFunctor>();
        auto index = static_cast<size_t>(type);
        return table.at(index)(*this);
    }
    
    size_t ScriptAccess::totalAddressCount() const {
        uint32_t count = 0;
        for_each(scriptFiles, [&count](auto& obj) -> decltype(auto) {count += obj->size();});
        return count;
    }
} // namespace blocksci
