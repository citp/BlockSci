//
//  address_index.cpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_index.hpp"
#include "chain/transaction.hpp"
#include "chain/inout_pointer.hpp"
#include "chain/output.hpp"
#include "chain/input.hpp"
#include "address/address.hpp"
#include "address/address_info.hpp"
#include "scripts/script_info.hpp"
#include "scripts/script.hpp"

#include <range/v3/utility/optional.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/action/transform.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/to_container.hpp>

#include <unordered_set>
#include <vector>
#include <sstream>
#include <iostream>

namespace blocksci {
    
    std::vector<Output> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Input> getInputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getOutputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getInputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    
    lmdb::env createAddressIndexEnviroment(const std::string &path) {
        auto env = lmdb::env::create();
        env.set_mapsize(40UL * 1024UL * 1024UL * 1024UL); /* 1 GiB */
        env.set_max_dbs(5);
        env.open(path.c_str(), MDB_NOSUBDIR, 0664);
        return env;
    }
    
    AddressIndex::AddressIndex(const std::string &path) : env(createAddressIndexEnviroment(path))  {
    }
    
    std::vector<Output> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        return pointers
        | ranges::view::transform([&](const OutputPointer &pointer) { return Output(pointer, access); })
        | ranges::to_vector;
    }
    
    std::vector<Output> AddressIndex::getOutputs(const Address &address, const ChainAccess &access) const {
        return getOutputsImp(getOutputPointers(address), access);
    }
    
    std::vector<Output> AddressIndex::getOutputs(const Script &script, const ChainAccess &access) const {
        return getOutputsImp(getOutputPointers(script), access);
    }
    
    std::vector<Input> getInputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        std::unordered_set<InputPointer> allPointers;
        allPointers.reserve(pointers.size());
        for (auto &pointer : pointers) {
            auto inputTx = Output(pointer, access).getSpendingTx();
            if(inputTx) {
                auto inputPointers = inputTx->getInputPointers(pointer);
                for (auto &inputPointer : inputPointers) {
                    allPointers.insert(inputPointer);
                }
            }
        }
        return allPointers
        | ranges::view::transform([&](const InputPointer &pointer) { return Input(pointer, access); })
        | ranges::to_vector;
    }
    
    std::vector<Input> AddressIndex::getInputs(const Address &address, const ChainAccess &access) const {
        return getInputsImp(getOutputPointers(address), access);
    }
    
    std::vector<Input> AddressIndex::getInputs(const Script &script, const ChainAccess &access) const {
        return getInputsImp(getOutputPointers(script), access);
    }
    
    std::vector<Transaction> getTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size() * 2);
        for (auto &pointer : pointers) {
            txes.insert(Transaction(pointer.txNum, access));
            auto inputTx = Output(pointer, access).getSpendingTx();
            if (inputTx) {
                txes.insert(*inputTx);
            }
        }
        return {txes.begin(), txes.end()};
    }
    
    std::vector<Transaction> AddressIndex::getTransactions(const Address &address, const ChainAccess &access) const {
        return getTransactionsImp(getOutputPointers(address), access);
    }
    
    std::vector<Transaction> AddressIndex::getTransactions(const Script &script, const ChainAccess &access) const {
        return getTransactionsImp(getOutputPointers(script), access);
    }
    
    std::vector<Transaction> getOutputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        auto txNums = pointers | ranges::view::transform([](const OutputPointer &pointer) { return pointer.txNum; }) | ranges::to_vector;
        txNums |= ranges::action::sort | ranges::action::unique;
        return txNums | ranges::view::transform([&](uint32_t txNum) { return Transaction(txNum, access); }) | ranges::to_vector;
    }
    
    std::vector<Transaction> AddressIndex::getOutputTransactions(const Address &address, const ChainAccess &access) const {
        return getOutputTransactionsImp(getOutputPointers(address), access);
    }
    
    std::vector<Transaction> AddressIndex::getOutputTransactions(const Script &script, const ChainAccess &access) const {
        return getOutputTransactionsImp(getOutputPointers(script), access);
    }
    
    
    auto flatMap() {
        return ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
        | ranges::view::transform([](const auto &optional) { return *optional; });
    }
    
    std::vector<Transaction> getInputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        auto txes = pointers | ranges::view::transform([&](const OutputPointer &pointer) { return Output(pointer, access).getSpendingTx(); }) | flatMap() | ranges::to_vector;
        txes |= ranges::action::sort(operator<) | ranges::action::unique(operator==);
        return txes;
    }
    
    std::vector<Transaction> AddressIndex::getInputTransactions(const Address &address, const ChainAccess &access) const {
        return getInputTransactionsImp(getOutputPointers(address), access);
    }
    
    std::vector<Transaction> AddressIndex::getInputTransactions(const Script &script, const ChainAccess &access) const {
        return getInputTransactionsImp(getOutputPointers(script), access);
    }
    
    template<typename Query>
    std::vector<OutputPointer> getOutputPointersImp(lmdb::cursor &cursor, lmdb::val &key, const Query &query) {
        std::vector<OutputPointer> pointers;
        lmdb::val value;
        for (bool hasNext = cursor.get(key, value, MDB_SET_RANGE); hasNext; hasNext = cursor.get(key, value, MDB_NEXT)) {
            Address *address = key.data<Address>();
            if (*address != query) {
                break;
            }
            OutputPointer *pointer = value.data<OutputPointer>();
            pointers.push_back(*pointer);
        }
        return pointers;
    }
                                                                      
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &searchAddress) const {
        std::vector<OutputPointer> pointers;
        auto rtxn = lmdb::txn::begin(env, nullptr, MDB_RDONLY);
        auto dbi = lmdb::dbi::open(rtxn, std::string(scriptName(scriptType(searchAddress.type))).c_str(), MDB_DUPSORT| MDB_DUPFIXED);
        auto cursor = lmdb::cursor::open(rtxn, dbi);
        lmdb::val key{&searchAddress, sizeof(searchAddress)};
        return getOutputPointersImp(cursor, key, searchAddress);
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Script &script) const {
        std::vector<OutputPointer> pointers;
        auto rtxn = lmdb::txn::begin(env, nullptr, MDB_RDONLY);
        auto dbi = lmdb::dbi::open(rtxn, std::string(scriptName(script.type)).c_str(), MDB_DUPSORT| MDB_DUPFIXED);
        auto cursor = lmdb::cursor::open(rtxn, dbi);
        lmdb::val key{&script.scriptNum, sizeof(script.scriptNum)};
        return getOutputPointersImp(cursor, key, script);
    }
}

