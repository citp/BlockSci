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
    
    AddressIndex::AddressIndex(const std::string &path)  {
        rocksdb::Options options;
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        options.create_missing_column_families = true;
        
        
        std::vector<rocksdb::ColumnFamilyDescriptor> columnDescriptors;
        columnDescriptors.emplace_back(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());
        for (auto script : ScriptType::all) {
            columnDescriptors.push_back(rocksdb::ColumnFamilyDescriptor{scriptName(script), rocksdb::ColumnFamilyOptions{}});
        }
        rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.c_str(), columnDescriptors, &columnHandles, &db);
        assert(s.ok());
    }
    
    AddressIndex::~AddressIndex() {
        for (auto handle : columnHandles) {
            delete handle;
        }
        delete db;
    }
    
    std::vector<Output> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        auto chainAccess = &access;
        return pointers
        | ranges::view::transform([chainAccess](const OutputPointer &pointer) { return Output(pointer, *chainAccess); })
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
        auto chainAccess = &access;
        return allPointers
        | ranges::view::transform([chainAccess](const InputPointer &pointer) { return Input(pointer, *chainAccess); })
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
        auto chainAccess = &access;
        return txNums | ranges::view::transform([chainAccess](uint32_t txNum) { return Transaction(txNum, *chainAccess); }) | ranges::to_vector;
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
        auto chainAccess = &access;
        auto txes = pointers | ranges::view::transform([chainAccess](const OutputPointer &pointer) { return Output(pointer, *chainAccess).getSpendingTx(); }) | flatMap() | ranges::to_vector;
        txes |= ranges::action::sort | ranges::action::unique;
        return txes;
    }
    
    std::vector<Transaction> AddressIndex::getInputTransactions(const Address &address, const ChainAccess &access) const {
        return getInputTransactionsImp(getOutputPointers(address), access);
    }
    
    std::vector<Transaction> AddressIndex::getInputTransactions(const Script &script, const ChainAccess &access) const {
        return getInputTransactionsImp(getOutputPointers(script), access);
    }
    
//    auto column = columnHandles[static_cast<size_t>(ScriptType::SCRIPTHASH) + 1];
    
    std::vector<OutputPointer> getOutputPointersImp(rocksdb::DB *db, rocksdb::ColumnFamilyHandle *column, const rocksdb::Slice &key) {
        std::vector<OutputPointer> pointers;
        rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), column);
        for (it->Seek(key); it->Valid() && it->key().starts_with(key); it->Next()) {
            auto foundKey = it->key();
            foundKey.remove_prefix(sizeof(Address));
            OutputPointer outPoint;
            memcpy(&outPoint, foundKey.data(), sizeof(outPoint));
            pointers.push_back(outPoint);
        }
        return pointers;
    }
                                                                      
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &searchAddress) const {
        auto column = columnHandles[static_cast<size_t>(scriptType(searchAddress.type)) + 1];
        rocksdb::Slice key{reinterpret_cast<const char *>(&searchAddress), sizeof(searchAddress)};
        return getOutputPointersImp(db, column, key);
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Script &script) const {
        auto column = columnHandles[static_cast<size_t>(script.type) + 1];
        rocksdb::Slice key{reinterpret_cast<const char *>(&script.scriptNum), sizeof(script.scriptNum)};
        return getOutputPointersImp(db, column, key);
    }
}

