//
//  script_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef script_access_hpp
#define script_access_hpp

#include "script_info.hpp"
#include "script_data.hpp"

#include <blocksci/address/dedup_address_info.hpp>
#include <blocksci/util/data_configuration.hpp>
#include <blocksci/util/file_mapper.hpp>
#include <blocksci/util/dynamic_table.hpp>
#include <blocksci/util/apply.hpp>
#include <blocksci/util/for_each.hpp>

#include <mpark/variant.hpp>


namespace blocksci {
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
    
    class ScriptAccess {
    private:
        using ScriptFilesTuple = to_dedup_address_tuple_t<ScriptFilePtr>;
        ScriptFilesTuple scriptFiles;
        
        
    public:
        explicit ScriptAccess(const DataConfiguration &config_) :
        scriptFiles(blocksci::apply(DedupAddressType::all(), [&] (auto tag) {
            return std::make_unique<ScriptFile<tag.value>>(config_.scriptsDirectory()/ std::string{dedupAddressName(tag)});
        })), config(config_) {}
        
        DataConfiguration config;
        
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
            return getFile<type>().getData(addressNum - 1);
        }
        
        template<DedupAddressType::Enum type>
        size_t scriptCount() const {
            return getFile<type>().size();
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
            return static_cast<uint32_t>(access.scriptCount<type>());
        }
    } // namespace internal
} // namespace blocksci

#endif /* script_access_hpp */
