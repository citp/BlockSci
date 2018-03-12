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

using namespace blocksci;

AddressDB::AddressDB(const ParserConfigurationBase &config_, const std::string &path) : ParserIndex(config_, "addressDB"), db(path, false) {}

void AddressDB::tearDown() {}

void AddressDB::processTx(const blocksci::Transaction &tx) {
    std::unordered_set<Address> addresses;
    std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
        if (dedupType(a.type) == DedupAddressType::SCRIPTHASH) {
            script::ScriptHash scriptHash(a.scriptNum, tx.getAccess());
            if (scriptHash.getTxRevealedIndex() == tx.txNum) {
                auto wrapped = *scriptHash.getWrappedAddress();
                db.addAddressNested(wrapped, DedupAddress{a.scriptNum, DedupAddressType::SCRIPTHASH});
                return true;
            } else {
                return false;
            }
        }
        return false;
    };
    for (auto input : tx.inputs()) {
        visit(input.getAddress(), visitFunc);
    }
    
    for (auto output : tx.outputs()) {
        db.addAddressOutput(output.getAddress(), output.pointer);
    }
}

void AddressDB::rollback(const blocksci::State &state) {
    for_each(AddressInfoList(), [&](auto type) {
        auto column = db.getOutputColumn(type);
        auto it = db.getOutputIterator(type);
        rocksdb::WriteBatch batch;
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            auto key = it->key();
            key.remove_prefix(sizeof(uint32_t));
            OutputPointer outPoint;
            memcpy(&outPoint, key.data(), sizeof(outPoint));
            if (outPoint.txNum >= state.scriptCounts[static_cast<size_t>(blocksci::DedupAddressType::SCRIPTHASH)]) {
                batch.Delete(column, it->key());
            }
        }
        assert(it->status().ok()); // Check for any errors found during the scan
        delete it;
    });
}
