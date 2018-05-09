//
//  script_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef script_access_hpp
#define script_access_hpp

#include <blocksci/core/file_mapper.hpp>
#include <blocksci/core/script_data.hpp>
#include <blocksci/core/script_info.hpp>

#include <blocksci/blocksci_export.h>
#include <blocksci/meta.hpp>

#include <memory>
#include <tuple>

namespace blocksci {
    
    template<typename T>
    struct ScriptFileType;
    
    template<typename T>
    struct ScriptFileType<FixedSize<T>> {
        using type = FixedSizeFileMapper<T>;
    };
    
    template<typename ...T>
    struct ScriptFileType<Indexed<T...>> {
        using type = IndexedFileMapper<mio::access_mode::read, T...>;
    };
    
    template<typename T>
    using ScriptFileType_t = typename ScriptFileType<T>::type;
    
    
    template<DedupAddressType::Enum type>
    using ScriptFile = ScriptFileType_t<typename ScriptInfo<type>::storage>;
    
    template<DedupAddressType::Enum type>
    struct ScriptDataBaseFunctor {
        static const ScriptDataBase * f(uint32_t scriptNum, const ScriptAccess &access);
    };
    
    class BLOCKSCI_EXPORT ScriptAccess {
    private:
        using ScriptFilesTuple = to_dedup_address_tuple_t<ScriptFile>;
        ScriptFilesTuple scriptFiles;
        
    public:
        explicit ScriptAccess(const std::string &baseDirectory);
        
        template <DedupAddressType::Enum type>
        ScriptFile<type> &getFile() {
            return *std::get<ScriptFile<type>>(scriptFiles);
        }
        
        template <DedupAddressType::Enum type>
        const ScriptFile<type> &getFile() const {
            return std::get<ScriptFile<type>>(scriptFiles);
        }
        
        template <DedupAddressType::Enum type>
        auto getScriptData(uint32_t addressNum) const {
            return getFile<type>()[addressNum - 1];
        }
        
        const ScriptDataBase *getScriptHeader(uint32_t addressNum, DedupAddressType::Enum type) const {
            static auto &scriptDataBaseTable = *[]() {
                auto table = make_dynamic_table<DedupAddressType, ScriptDataBaseFunctor>();
                return new decltype(table){table};
            }();
            auto index = static_cast<size_t>(type);
            return scriptDataBaseTable.at(index)(addressNum, *this);
        }
        
        std::array<uint32_t, DedupAddressType::size> scriptCounts() const;
        
        uint32_t scriptCount(DedupAddressType::Enum type) const;
        
        size_t totalAddressCount() const;
        
        void reload() {
            for_each(scriptFiles, [&](auto& file) -> decltype(auto) { file.reload(); });
        }
    };
    
    template<DedupAddressType::Enum type>
    const ScriptDataBase * ScriptDataBaseFunctor<type>::f(uint32_t scriptNum, const ScriptAccess &access) {
        auto &file = access.getFile<type>();
        return file.getDataAtIndex(scriptNum - 1);
    }
    
} // namespace blocksci

#endif /* script_access_hpp */
