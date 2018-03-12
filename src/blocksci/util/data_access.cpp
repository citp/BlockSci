//
//  data_access.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/18/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "util/data_access.hpp"
#include "util/data_configuration.hpp"

#include <blocksci/scripts/script_access.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/action/sort.hpp>

#include <unordered_set>

namespace blocksci {
    
    DataAccess::DataAccess(const DataConfiguration &config_) : config(config_), chain{std::make_unique<ChainAccess>(config)}, scripts{std::make_unique<ScriptAccess>(config)}, addressIndex{std::make_unique<AddressIndex>(config.addressDBFilePath().native(), true)}, hashIndex{std::make_unique<HashIndex>(config.hashIndexFilePath().native(), true)} {}
    
    DataAccess &DataAccess::Instance2(const DataConfiguration &config_) {
        // Since it's a static variable, if the class has already been created,
        // It won't be created again.
        // And it **is** thread-safe in C++11.
        
        static DataAccess myInstance(config_);
        static DataConfiguration config = config_;
        
        if (!config_.isNull()) {
            config = config_;
            myInstance.chain->reload();
            myInstance.scripts->reload();
        }
        
        // Return a reference to our instance.
        return myInstance;
    }
    
    std::vector<Output> getOutputsImp(std::vector<OutputPointer> pointers, const DataAccess *access) {
        return pointers
        | ranges::view::transform([access](const OutputPointer &pointer) { return Output(pointer, *access); })
        | ranges::to_vector;
    }
    
    std::vector<Output> DataAccess::getOutputs(const Address &address) const {
        return getOutputsImp(addressIndex->getOutputPointers(address), this);
    }
    
    std::vector<Output> DataAccess::getOutputs(const EquivAddress &script) const {
        return getOutputsImp(addressIndex->getOutputPointers(script), this);
    }
    
    std::vector<Input> getInputsImp(std::vector<OutputPointer> pointers, const DataAccess *access) {
        std::unordered_set<InputPointer> allPointers;
        allPointers.reserve(pointers.size());
        for (auto &pointer : pointers) {
            auto inputTx = Output(pointer, *access).getSpendingTx();
            if(inputTx) {
                auto inputPointers = inputTx->getInputPointers(pointer);
                for (auto &inputPointer : inputPointers) {
                    allPointers.insert(inputPointer);
                }
            }
        }
        return allPointers
        | ranges::view::transform([access](const InputPointer &pointer) { return Input(pointer, *access); })
        | ranges::to_vector;
    }
    
    std::vector<Input> DataAccess::getInputs(const Address &address) const {
        return getInputsImp(addressIndex->getOutputPointers(address), this);
    }
    
    std::vector<Input> DataAccess::getInputs(const EquivAddress &script) const {
        return getInputsImp(addressIndex->getOutputPointers(script), this);
    }
    
    std::vector<Transaction> getTransactionsImp(std::vector<OutputPointer> pointers, const DataAccess *access) {
        
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size() * 2);
        for (auto &pointer : pointers) {
            txes.insert(Transaction(pointer.txNum, *access));
            auto inputTx = Output(pointer, *access).getSpendingTx();
            if (inputTx) {
                txes.insert(*inputTx);
            }
        }
        return {txes.begin(), txes.end()};
    }
    
    std::vector<Transaction> DataAccess::getTransactions(const Address &address) const {
        return getTransactionsImp(addressIndex->getOutputPointers(address), this);
    }
    
    std::vector<Transaction> DataAccess::getTransactions(const EquivAddress &script) const {
        return getTransactionsImp(addressIndex->getOutputPointers(script), this);
    }
    
    std::vector<Transaction> getOutputTransactionsImp(std::vector<OutputPointer> pointers, const DataAccess *access) {
        auto txNums = pointers | ranges::view::transform([](const OutputPointer &pointer) { return pointer.txNum; }) | ranges::to_vector;
        txNums |= ranges::action::sort | ranges::action::unique;
        return txNums | ranges::view::transform([access](uint32_t txNum) { return Transaction(txNum, *access); }) | ranges::to_vector;
    }
    
    std::vector<Transaction> DataAccess::getOutputTransactions(const Address &address) const {
        return getOutputTransactionsImp(addressIndex->getOutputPointers(address), this);
    }
    
    std::vector<Transaction> DataAccess::getOutputTransactions(const EquivAddress &script) const {
        return getOutputTransactionsImp(addressIndex->getOutputPointers(script), this);
    }
    
    
    auto flatMap() {
        return ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
        | ranges::view::transform([](const auto &optional) { return *optional; });
    }
    
    std::vector<Transaction> getInputTransactionsImp(std::vector<OutputPointer> pointers, const DataAccess *access) {
        auto txes = pointers | ranges::view::transform([access](const OutputPointer &pointer) { return Output(pointer, *access).getSpendingTx(); }) | flatMap() | ranges::to_vector;
        txes |= ranges::action::sort | ranges::action::unique;
        return txes;
    }
    
    std::vector<Transaction> DataAccess::getInputTransactions(const Address &address) const {
        return getInputTransactionsImp(addressIndex->getOutputPointers(address), this);
    }
    
    std::vector<Transaction> DataAccess::getInputTransactions(const EquivAddress &script) const {
        return getInputTransactionsImp(addressIndex->getOutputPointers(script), this);
    }
}



