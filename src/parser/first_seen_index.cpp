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

FirstSeenIndex::FirstSeenIndex(const ParserConfiguration &config) : AddressTraverser(config, "first_seen") {}

void FirstSeenIndex::prepareUpdate(const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    auto tags = blocksci::ScriptInfoList();
    files.clear();
    blocksci::for_each(tags, [&](auto tag) -> decltype(auto) {
        constexpr auto type = decltype(tag)::type;
        auto path = config.firstSeenDirectory()/blocksci::ScriptInfo<type>::name;
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
        
        boost::filesystem::resize_file(fullPath, sizeof(uint32_t) * scripts.scriptCount<type>());
        
        files.emplace(std::piecewise_construct, std::forward_as_tuple(type), std::forward_as_tuple(path));
    });
}

void FirstSeenIndex::maybeUpdate(const blocksci::Address &address, uint32_t txNum) {
    auto type = scriptType(address.type);
    auto it = files.find(type);
    auto &file = it->second;
    auto oldValue = *file.getData(address.addressNum - 1);
    if (oldValue == 0 || txNum < oldValue) {
        file.update(address.addressNum - 1, txNum);
    }
}

void FirstSeenIndex::sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) {
    maybeUpdate(address, pointer.txNum);
}

void FirstSeenIndex::revealedP2SH(uint32_t scriptNum, const blocksci::Address &wrappedAddress, const blocksci::ScriptAccess &scripts) {
    auto &p2shFile = files.find(blocksci::ScriptType::Enum::SCRIPTHASH)->second;
    auto firstUsage = *p2shFile.getData(scriptNum - 1);
    std::function<void(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
        maybeUpdate(a, firstUsage);
    };
    visit(wrappedAddress, visitFunc, scripts);
}
