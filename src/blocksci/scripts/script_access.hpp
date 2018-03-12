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

#include <blocksci/util/data_configuration.hpp>
#include <blocksci/util/file_mapper.hpp>
#include <blocksci/address/address_info.hpp>

#include <mpark/variant.hpp>

namespace blocksci {
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
        using ScriptFilesTuple = to_equiv_address_tuple_t<ScriptFilePtr>;
        ScriptFilesTuple scriptFiles;
        
        
    public:
        ScriptAccess(const DataConfiguration &config);
        
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
        
        template <AddressType::Enum type>
        auto getScriptData(uint32_t addressNum) const {
            return getScriptData<dedupType(type)>(addressNum);
        }
        
        template<DedupAddressType::Enum type>
        size_t scriptCount() const {
            return getFile<type>().size();
        }
        
        std::array<uint32_t, DedupAddressType::size> scriptCounts() const;
        uint32_t scriptCount(DedupAddressType::Enum type) const;
        
        size_t totalAddressCount() const;
        
        void reload();
    };

}

#endif /* script_access_hpp */
