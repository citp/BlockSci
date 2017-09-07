//
//  first_seen_index.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/29/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "first_seen_index.hpp"
#include "parser_configuration.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address_info.hpp>


#include <blocksci/data_access.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>

FirstSeenIndex::FirstSeenIndex(const ParserConfiguration &config, const blocksci::ScriptAccess &access) {
    auto tags = blocksci::ScriptInfoList();
    blocksci::for_each(tags, [this, &config, &access](auto tag) -> decltype(auto) {
        constexpr auto type = decltype(tag)::type;
        auto path = config.firstSeenDirectory()/blocksci::ScriptInfo<type>::typeName;
        auto mainParams = std::fstream::out | std::fstream::binary;
        auto extraParams = std::fstream::ate | std::fstream::in;
        
        auto fullPath = path;
        fullPath.concat(".dat");
        boost::filesystem::fstream file(fullPath, mainParams | extraParams);
        if (!file.is_open()) {
            file.open(fullPath, mainParams);
        }
        if (file.is_open()) {
            file.close();
        }
        
        boost::filesystem::resize_file(fullPath, sizeof(uint32_t) * access.scriptCount<type>());
        
        this->files.emplace(std::piecewise_construct, std::forward_as_tuple(type), std::forward_as_tuple(path));
    });
}

void FirstSeenIndex::sawAddress(const blocksci::Address &pointer, uint32_t txNum) {
    auto type = scriptType(pointer.type);
    auto it = files.find(type);
    auto &file = it->second;
    auto oldValue = *file.getData(pointer.addressNum - 1);
    if (oldValue == 0 || txNum < oldValue) {
        file.update(pointer.addressNum - 1, txNum);
    }
}

void FirstSeenIndex::linkP2SHAddress(const blocksci::Address &pointer, uint32_t, uint32_t p2shNum) {
    
    auto &p2shFile = files.find(blocksci::ScriptType::Enum::SCRIPTHASH)->second;
    auto firstUsage = *p2shFile.getData(p2shNum);
    
    auto type = scriptType(pointer.type);
    auto it = files.find(type);
    auto &file = it->second;
    auto oldValue = *file.getData(pointer.addressNum - 1);
    if (oldValue == 0 || firstUsage < oldValue) {
        file.update(pointer.addressNum - 1, firstUsage);
    }
}
