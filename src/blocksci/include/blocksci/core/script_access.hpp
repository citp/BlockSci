//
//  script_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef script_access_hpp
#define script_access_hpp

#include "file_mapper.hpp"
#include "script_data.hpp"
#include "script_info.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/meta/dynamic_table.hpp>
#include <blocksci/meta/static_table.hpp>
#include <blocksci/meta/apply.hpp>

#include <memory>
#include <tuple>

namespace blocksci {
    struct DataConfiguration;

    namespace internal {
        template<DedupAddressType::Enum type>
        struct ScriptCountFunctor {
            static uint32_t f(const ScriptAccess &access);
        };
    } // namespace internal
    
    template<typename T>
    struct ScriptFileType;
    
    template<typename T>
    struct ScriptFileType<FixedSize<T>> {
        using type = FixedSizeFileMapper<T>;
    };
    
    template<typename ...T>
    struct ScriptFileType<Indexed<T...>> {
        using type = IndexedFileMapper<AccessMode::readonly, T...>;
    };
    
    template<typename T>
    using ScriptFileType_t = typename ScriptFileType<T>::type;
    
    
    template<DedupAddressType::Enum type>
    using ScriptFile = ScriptFileType_t<typename ScriptInfo<type>::storage>;
    
    template<DedupAddressType::Enum type>
    using ScriptFilePtr = std::unique_ptr<ScriptFile<type>>;
    
    class BLOCKSCI_EXPORT ScriptAccess {
    private:
        using ScriptFilesTuple = to_dedup_address_tuple_t<ScriptFilePtr>;
        ScriptFilesTuple scriptFiles;
        
        
    public:
        explicit ScriptAccess(const DataConfiguration &config_);
        
        template <DedupAddressType::Enum type>
        ScriptFile<type> &getFile() {
            return *std::get<ScriptFile<type>>(scriptFiles);
        }
        
        template <DedupAddressType::Enum type>
        const ScriptFile<type> &getFile() const {
            return *std::get<ScriptFilePtr<type>>(scriptFiles);
        }
        
        template <DedupAddressType::Enum type>
        auto getScriptData(uint32_t addressNum) const {
            return getFile<type>()[addressNum - 1];
        }
        
        template<DedupAddressType::Enum type>
        uint32_t scriptCount() const {
            return static_cast<uint32_t>(getFile<type>().size());
        }
        
        std::array<uint32_t, DedupAddressType::size> scriptCounts() const {
            return make_static_table<DedupAddressType, internal::ScriptCountFunctor>(*this);
        }
        
        uint32_t scriptCount(DedupAddressType::Enum type) const {
            static constexpr auto table = make_dynamic_table<DedupAddressType, internal::ScriptCountFunctor>();
            auto index = static_cast<size_t>(type);
            return table.at(index)(*this);
        }
        
        size_t totalAddressCount() const {
            uint32_t count = 0;
            for_each(scriptFiles, [&count](auto& obj) -> decltype(auto) {count += obj->size();});
            return count;
        }
        
        void reload() {
            for_each(scriptFiles, [&](auto& file) -> decltype(auto) { file->reload(); });
        }
    };
    
    namespace internal {
        template<DedupAddressType::Enum type>
        uint32_t ScriptCountFunctor<type>::f(const ScriptAccess &access) {
            return access.scriptCount<type>();
        }
    } // namespace internal
} // namespace blocksci

#endif /* script_access_hpp */
