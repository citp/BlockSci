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

#include <vector>
#include <sstream>
#include <iostream>

namespace blocksci {
    
    template<AddressType::Enum type>
    struct AddressQueryFunctor {
        static sqlite3_stmt *f(sqlite3 *db) {
            sqlite3_stmt *stmt;
            std::stringstream ss;
            ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << scriptName(scriptType(type)) << " WHERE ADDRESS_NUM = ? AND ADDRESS_TYPE = ?";
            auto rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, 0);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                return nullptr;
            }
            sqlite3_bind_int(stmt, 2, static_cast<int>(type));
            return stmt;
        }
    };
    
    template<ScriptType::Enum type>
    struct ScriptQueryFunctor {
        static sqlite3_stmt *f(sqlite3 *db) {
            sqlite3_stmt *stmt;
            std::stringstream ss;
            ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << scriptName(type) << " WHERE ADDRESS_NUM = ?";
            auto rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, 0);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                return nullptr;
            }
            return stmt;
        }
    };
    
    sqlite3 *openAddressDb(const char *filename) {
        sqlite3 *db;
        auto rc = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        }
        return db;
    }
    
    AddressIndex::AddressIndex(const char *filename) : addressDb(openAddressDb(filename)), addressQueries(make_static_table<AddressType, AddressQueryFunctor>(addressDb)), scriptQueries(make_static_table<ScriptType, ScriptQueryFunctor>(addressDb)) {}
    
    AddressIndex::AddressIndex(const DataConfiguration &config) : AddressIndex(config.addressDBFilePath().c_str())  {}
    
    
    AddressIndex::AddressIndex(const AddressIndex &other) : AddressIndex(sqlite3_db_filename(other.addressDb, "main")) {}
    
    AddressIndex::~AddressIndex() {
        for_each(addressQueries, [](auto &a) { sqlite3_finalize(a); });
        for_each(scriptQueries, [](auto &a) { sqlite3_finalize(a); });
        sqlite3_close(addressDb);
    }
    
    
    AddressIndex& AddressIndex::operator=(const AddressIndex& other) {
        sqlite3_close(addressDb);
        addressDb = openAddressDb(sqlite3_db_filename(other.addressDb, "main"));
        return *this;
    }
    
    std::vector<const blocksci::Output *> getOutputsImp(std::vector<OutputPointer> pointers, const ChainAccess &access) {
        std::vector<const Output *> outputs;
        outputs.reserve(pointers.size());
        for (auto &pointer : pointers) {
            outputs.push_back(&pointer.getOutput(access));
        }
        return outputs;
    }
    
    std::vector<const blocksci::Output *> AddressIndex::getOutputs(const Address &address, const ChainAccess &access) const {
        return getOutputsImp(getOutputPointers(address), access);
    }
    
    std::vector<const blocksci::Output *> AddressIndex::getOutputs(const Script &script, const ChainAccess &access) const {
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
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Address &address) const {
        std::vector<OutputPointer> outputs;
        
        auto stmt = addressQueries[static_cast<size_t>(address.type)];
        sqlite3_bind_int(stmt, 1, static_cast<int32_t>(address.addressNum));
        
        int rc = 0;
        while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            auto txNum = sqlite3_column_int64(stmt, 0);
            auto outputNum = sqlite3_column_int(stmt, 1);
            outputs.push_back({static_cast<uint32_t>(txNum), static_cast<uint16_t>(outputNum)});
        }
        return outputs;
    }
    
    std::vector<OutputPointer> AddressIndex::getOutputPointers(const Script &script) const {
        std::vector<OutputPointer> outputs;
        
        auto stmt = scriptQueries[static_cast<size_t>(script.type())];
        sqlite3_bind_int(stmt, 1, static_cast<int32_t>(script.scriptNum));
        
        int rc = 0;
        while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            auto txNum = sqlite3_column_int64(stmt, 0);
            auto outputNum = sqlite3_column_int(stmt, 1);
            outputs.emplace_back(static_cast<uint32_t>(txNum), static_cast<uint16_t>(outputNum));
        }
        
        return outputs;
    }
}

