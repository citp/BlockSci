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
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_access.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>

FirstSeenIndex::FirstSeenIndex(const ParserConfigurationBase &config) : AddressTraverser(config, "first_seen") {
    for (auto type : blocksci::ScriptType::all) {
        auto path = config.firstSeenDirectory()/std::string{scriptName(type)};
        files.emplace(std::piecewise_construct, std::forward_as_tuple(type), std::forward_as_tuple(path));
    }
}

void FirstSeenIndex::prepareUpdate() {
    blocksci::ScriptAccess scripts{config};
    for (auto &pair : files) {
        auto &file = pair.second;
        file.truncate(scripts.scriptCount(pair.first));
    }
}

// truncate

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

void FirstSeenIndex::revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) {
    auto &p2shFile = files.find(blocksci::ScriptType::Enum::SCRIPTHASH)->second;
    auto firstUsage = *p2shFile.getData(scriptHash.scriptNum - 1);
    std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
        maybeUpdate(a, firstUsage);
        return true;
    };
    visit(*scriptHash.getWrappedAddress(), visitFunc, scripts);
}
