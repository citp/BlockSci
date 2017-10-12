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
    
    template<AddressType::Enum type>
    struct AddressQueryFunctor {
        static SQLite::Statement f(SQLite::Database &db) {
            std::stringstream ss;
            ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << scriptName(scriptType(type)) << " WHERE ADDRESS_NUM = ? AND ADDRESS_TYPE = ";
            ss << static_cast<size_t>(type);
            return SQLite::Statement{db, ss.str()};
        }
    };
    
    template<ScriptType::Enum type>
    struct ScriptQueryFunctor {
        static SQLite::Statement f(SQLite::Database &db) {
            std::stringstream ss;
            ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << scriptName(type) << " WHERE ADDRESS_NUM = ?";
            return SQLite::Statement{db, ss.str()};
        }
    };
    
    std::vector<const Output *> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<const Input *> getInputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getOutputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    std::vector<Transaction> getInputTransactionsImp(std::vector<OutputPointer> pointers, const ChainAccess &access);
    
    AddressIndex::AddressIndex(const DataConfiguration &config) : db(config.addressDBFilePath().native()), addressQueries( make_static_table<AddressType, AddressQueryFunctor>(db)), scriptQueries(make_static_table<ScriptType, ScriptQueryFunctor>(db))  {
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
    
    std::vector<OutputPointer> getOutputPointersImp(SQLite::Statement &query, uint32_t scriptNum) {
        std::vector<OutputPointer> outputs;
        query.bind(1, scriptNum);
        
        while (query.executeStep()) {
            auto txNum = query.getColumn(0).getUInt();
            auto outputNum = static_cast<uint16_t>(query.getColumn(0).getUInt());
            outputs.emplace_back(txNum, outputNum);
        }
        query.reset();
        return outputs;
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &address) const {
        auto &query = addressQueries[static_cast<size_t>(address.type)];
        return getOutputPointersImp(query, address.addressNum);
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Script &script) const {
        auto scriptType = script.type();
        auto &query = scriptQueries[static_cast<size_t>(scriptType)];
        return getOutputPointersImp(query, script.scriptNum);
    }
}

