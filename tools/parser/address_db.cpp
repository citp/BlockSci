//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_db.hpp"
#include "raw_address_visitor.hpp"

#include <blocksci/core/address_type_meta.hpp>
#include <blocksci/core/inout_pointer.hpp>

#include <internal/address_info.hpp>

using blocksci::RawAddress;
using blocksci::DedupAddress;
using blocksci::InoutPointer;
using blocksci::State;
using blocksci::DedupAddressType;

AddressDB::AddressDB(const ParserConfigurationBase &config_, const filesystem::path &path) : ParserIndex(config_, "addressDB"), db(path, false) {
    outputCache.reserve(cacheSize);
    nestedCache.reserve(cacheSize);
}

AddressDB::~AddressDB() {
    clearNestedCache();
    clearOutputCache();
}

void AddressDB::processTx(const blocksci::RawTransaction *tx, uint32_t txNum, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    std::unordered_set<RawAddress> addedAddresses;
    std::function<bool(const RawAddress &)> visitFunc = [&](const RawAddress &a) {
        if (dedupType(a.type) == DedupAddressType::SCRIPTHASH && addedAddresses.find(a) == addedAddresses.end()) {
            addedAddresses.insert(a);
            auto scriptHash = scripts.getScriptData<DedupAddressType::SCRIPTHASH>(a.scriptNum);
            if (scriptHash->txFirstSpent == txNum) {
                addAddressNested(scriptHash->wrappedAddress, DedupAddress{a.scriptNum, DedupAddressType::SCRIPTHASH});
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    };
    auto inputs = ranges::make_subrange(tx->beginInputs(), tx->endInputs());
    for (auto &input : inputs) {
        visit(RawAddress{input.getAddressNum(), input.getType()}, visitFunc, scripts);
    }
    
    for (uint16_t i = 0; i < tx->outputCount; i++) {
        auto &output = tx->getOutput(i);
        auto pointer = InoutPointer{txNum, i};
        addAddressOutput(blocksci::RawAddress{output.getAddressNum(), output.getType()}, pointer);
    }
}

void AddressDB::addAddressNested(const blocksci::RawAddress &childAddress, const blocksci::DedupAddress &parentAddress) {
    nestedCache.emplace_back(childAddress, parentAddress);
    if (nestedCache.size() >= cacheSize) {
        clearNestedCache();
    }
}

void AddressDB::clearNestedCache() {
    db.addNestedAddresses(std::move(nestedCache));
    nestedCache.clear();
}

void AddressDB::addAddressOutput(const blocksci::RawAddress &address, const blocksci::InoutPointer &pointer) {
    outputCache.emplace_back(address, pointer);
    if (outputCache.size() >= cacheSize) {
        clearOutputCache();
    }
}

void AddressDB::clearOutputCache() {
    db.addOutputAddresses(std::move(outputCache));
    outputCache.clear();
}
