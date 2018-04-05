//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_db.hpp"

#include <blocksci/index/address_index.hpp>
#include <blocksci/address/dedup_address.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_access.hpp>

#include <unordered_set>
#include <string>
#include <iostream>

using blocksci::Address;
using blocksci::RawAddress;
using blocksci::DedupAddress;
using blocksci::AddressInfoList;
using blocksci::Transaction;
using blocksci::OutputPointer;
using blocksci::State;
using blocksci::DedupAddressType;
using blocksci::script::ScriptHash;

AddressDB::AddressDB(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "addressDB"), db(path, false) {}

void AddressDB::tearDown() {}

void AddressDB::processTx(const blocksci::RawTransaction *tx, uint32_t txNum, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    std::unordered_set<Address> addresses;
    std::function<bool(const RawAddress &)> visitFunc = [&](const RawAddress &a) {
        if (dedupType(a.type) == DedupAddressType::SCRIPTHASH) {
            auto scriptHash = scripts.getScriptData<DedupAddressType::SCRIPTHASH>(a.scriptNum);
            if (scriptHash->txFirstSeen == txNum) {
                db.addAddressNested(scriptHash->wrappedAddress, DedupAddress{a.scriptNum, DedupAddressType::SCRIPTHASH});
                return true;
            } else {
                return false;
            }
        }
        return false;
    };
    for (auto &input : tx->inputs()) {
        visit(RawAddress{input.toAddressNum, input.getType()}, visitFunc, scripts);
    }
    
    for (uint16_t i = 0; i < tx->outputCount; i++) {
        auto &output = tx->getOutput(i);
        auto pointer = OutputPointer{txNum, i};
        db.addAddressOutput(RawAddress{output.toAddressNum, output.getType()}, pointer);
    }
}

void AddressDB::rollback(const State &state) {
    for_each(AddressInfoList(), [&](auto type) {
        auto &column = db.getOutputColumn(type);
        auto it = db.getOutputIterator(type);
        rocksdb::WriteBatch batch;
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            auto key = it->key();
            key.remove_prefix(sizeof(uint32_t));
            OutputPointer outPoint;
            memcpy(&outPoint, key.data(), sizeof(outPoint));
            if (outPoint.txNum >= state.scriptCounts[static_cast<size_t>(blocksci::DedupAddressType::SCRIPTHASH)]) {
                batch.Delete(column.get(), it->key());
            }
        }
        assert(it->status().ok()); // Check for any errors found during the scan
    });
}
