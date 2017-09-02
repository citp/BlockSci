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

#include <blocksci/file_mapper.hpp>

#include <stdio.h>

namespace blocksci {
    
    struct DataConfiguration;
    
    template<typename T>
    struct ScriptFileType;
    
    template<typename T>
    struct ScriptFileType<blocksci::FixedSize<T>> {
        using type = FixedSizeFileMapper<T>;
    };
    
    template<typename ...T>
    struct ScriptFileType<blocksci::Indexed<T...>> {
        using type = IndexedFileMapper<boost::iostreams::mapped_file::mapmode::readonly, T...>;
    };
    
    template<typename T>
    using ScriptFileType_t = typename ScriptFileType<T>::type;
    
    
    template<ScriptType::Enum type>
    struct ScriptFile : public ScriptFileType_t<typename ScriptInfo<type>::storage> {
        using ScriptFileType_t<typename ScriptInfo<type>::storage>::ScriptFileType_t;
    };

    class ScriptAccess {
    private:
        using ScriptFilesTuple = internal::to_script_type<ScriptFile, ScriptInfoList>::type;
        
        ScriptFilesTuple scriptFiles;
        
        
    public:
        ScriptAccess(const DataConfiguration &config);
        
        template<ScriptType::Enum type>
        boost::optional<AddressPointer> findAddress(uint160 rawAddress) const {
            auto &file = std::get<ScriptFile<type>>(scriptFiles);
            auto index = file.find([&](const auto &item) { return item.address == rawAddress; });
            if (index) {
                return AddressPointer(*index + 1, type);
            }
            return boost::none;
        }
        
        template<ScriptType::Enum type, typename Func>
        std::vector<AddressPointer> findMatchingAddresses(Func func) const {
            auto &file = std::get<ScriptFile<type>>(scriptFiles);
            
            auto addressNums = file.findAll(func);
            
            std::vector<AddressPointer> addresses;
            addresses.reserve(addressNums.size());
            for (auto num : addressNums) {
                addresses.emplace_back(num + 1, type);
            }
            return addresses;
        }
        
        template <ScriptType::Enum type>
        auto getScriptData(uint32_t addressNum) const {
            return std::get<ScriptFile<type>>(scriptFiles).getData(addressNum - 1);
        }
        
        template<ScriptType::Enum type>
        size_t addressCount() const {
            auto &file = std::get<ScriptFile<type>>(scriptFiles);
            return file.size();
        }
        
        size_t addressCount(ScriptType::Enum type) const;
        
        size_t totalAddressCount() const;
    };

}

#endif /* script_access_hpp */
