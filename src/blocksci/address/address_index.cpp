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
#include "chain/output_pointer.hpp"
#include "chain/input_pointer.hpp"
#include "chain/output.hpp"
#include "chain/input.hpp"
#include "address.hpp"
#include "address_info.hpp"
#include "scripts/script_info.hpp"
#include "scripts/script.hpp"
#include "data_configuration.hpp"

#include <boost/optional/optional.hpp>

#include <vector>
#include <sstream>
#include <iostream>

namespace blocksci {
    
    std::vector<const Output *> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<const Input *> getInputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getOutputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getInputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    
    lmdb::env createAddressIndexEnviroment(const boost::filesystem::path &path) {
        auto env = lmdb::env::create();
        env.set_mapsize(100UL * 1024UL * 1024UL * 1024UL); /* 1 GiB */
        env.set_max_dbs(5);
        env.open(path.native().c_str(), MDB_NOSUBDIR, 0664);
        return env;
    }
    
    AddressIndex::AddressIndex(const DataConfiguration &config) : env(createAddressIndexEnviroment(config.addressDBFilePath()))  {
    }
    
    std::vector<const Output *> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        std::vector<const Output *> outputs;
        outputs.reserve(pointers.size());
        for (auto &pointer : pointers) {
            outputs.push_back(&pointer.getOutput(access));
        }
        return outputs;
    }
    
    std::vector<const Output *> AddressIndex::getOutputs(const Address &address, const ChainAccess &access) const {
        return getOutputsImp(getOutputPointers(address), access);
    }
    
    std::vector<const Output *> AddressIndex::getOutputs(const Script &script, const ChainAccess &access) const {
        return getOutputsImp(getOutputPointers(script), access);
    }
    
    std::vector<const Input *> getInputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        std::unordered_set<InputPointer> allPointers;
        allPointers.reserve(pointers.size());
        for (auto &pointer : pointers) {
            auto inputTx = pointer.getOutput(access).getSpendingTx(access);
            if(inputTx) {
                auto inputPointers = inputTx->getInputPointers(pointer, access);
                for (auto &inputPointer : inputPointers) {
                    allPointers.insert(inputPointer);
                }
            }
        }
        std::vector<const Input *> inputs;
        inputs.reserve(allPointers.size());
        for (auto &pointer : allPointers) {
            inputs.push_back(&pointer.getInput(access));
        }
        
        return inputs;
    }
    
    std::vector<const Input *> AddressIndex::getInputs(const Address &address, const ChainAccess &access) const {
        return getInputsImp(getOutputPointers(address), access);
    }
    
    std::vector<const Input *> AddressIndex::getInputs(const Script &script, const ChainAccess &access) const {
        return getInputsImp(getOutputPointers(script), access);
    }
    
    std::vector<Transaction> getTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size() * 2);
        for (auto &pointer : pointers) {
            txes.insert(pointer.getTransaction(access));
            auto inputTx = pointer.getOutput(access).getSpendingTx(access);
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
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size());
        for (auto &pointer : pointers) {
            txes.insert(pointer.getTransaction(access));
        }
        return {txes.begin(), txes.end()};
    }
    
    std::vector<Transaction> AddressIndex::getOutputTransactions(const Address &address, const ChainAccess &access) const {
        return getOutputTransactionsImp(getOutputPointers(address), access);
    }
    
    std::vector<Transaction> AddressIndex::getOutputTransactions(const Script &script, const ChainAccess &access) const {
        return getOutputTransactionsImp(getOutputPointers(script), access);
    }
    
    std::vector<Transaction> getInputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size());
        for (auto &pointer : pointers) {
            auto inputTx = pointer.getOutput(access).getSpendingTx(access);
            if (inputTx) {
                txes.insert(*inputTx);
            }
        }
        return {txes.begin(), txes.end()};
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

