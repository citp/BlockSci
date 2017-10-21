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

#include <blocksci/address/address_info.hpp>

#include <blocksci/file_mapper.hpp>

namespace blocksci {
    
    struct DataConfiguration;
    
    template<typename T>
    struct ScriptFileType;
    
    template<typename T>
    struct ScriptFileType<FixedSize<T>> {
        using type = FixedSizeFileMapper<T>;
    };
    
    template<typename ...T>
    struct ScriptFileType<Indexed<T...>> {
        using type = IndexedFileMapper<boost::iostreams::mapped_file::mapmode::readonly, T...>;
    };
    
    template<typename T>
    using ScriptFileType_t = typename ScriptFileType<T>::type;
    
    
    template<auto type>
    struct ScriptFile : public ScriptFileType_t<typename ScriptInfo<type>::storage> {
        using ScriptFileType_t<typename ScriptInfo<type>::storage>::ScriptFileType_t;
    };

    class ScriptAccess {
    private:
        using ScriptFilesTuple = apply_template_t<ScriptFile, ScriptInfoList>;
        
        ScriptFilesTuple scriptFiles;
        
        
    public:
        ScriptAccess(const DataConfiguration &config);
        
        template <ScriptType::Enum type>
        ScriptFile<type> &getFile() {
            return std::get<ScriptFile<type>>(scriptFiles);
        }
        
        template <ScriptType::Enum type>
        const ScriptFile<type> &getFile() const {
            return std::get<ScriptFile<type>>(scriptFiles);
        }
        
        template <ScriptType::Enum type>
        auto getScriptData(uint32_t addressNum) const {
            return getFile<type>().getData(addressNum - 1);
        }
        
        
        template<ScriptType::Enum type>
        size_t scriptCount() const {
            return getFile<type>().size();
        }
        
        std::array<uint32_t, ScriptType::size> scriptCounts() const;
        uint32_t scriptCount(ScriptType::Enum type) const;
        
        size_t totalAddressCount() const;
        
        void reload();
    };

}

#endif /* script_access_hpp */
