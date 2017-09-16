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
#include "data_configuration.hpp"

#include <vector>
#include <sstream>
#include <iostream>

namespace blocksci {
    
    AddressIndex::AddressIndex(const DataConfiguration &config) {
        auto rc = sqlite3_open_v2(config.addressDBFilePath().c_str(), &addressDb, SQLITE_OPEN_READONLY, NULL);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addressDb));
        }
    }
    
    AddressIndex::AddressIndex(const AddressIndex &other) {
        
        auto rc = sqlite3_open_v2(sqlite3_db_filename(other.addressDb, "main"), &addressDb, SQLITE_OPEN_READONLY, NULL);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addressDb));
        }
    }
    
    AddressIndex::~AddressIndex() {
        sqlite3_close(addressDb);
    }
    
    
    AddressIndex& AddressIndex::operator=(const AddressIndex& other) {
        sqlite3_close(addressDb);
        auto rc = sqlite3_open_v2(sqlite3_db_filename(other.addressDb, "main"), &addressDb, SQLITE_OPEN_READONLY, NULL);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addressDb));
        }
        return *this;
    }
    
    std::vector<const blocksci::Output *> AddressIndex::getOutputs(const blocksci::Address &address, const ChainAccess &access) const {
        auto pointers = getOutputPointers(address);
        std::vector<const Output *> outputs;
        outputs.reserve(pointers.size());
        for (auto &pointer : pointers) {
            outputs.push_back(&pointer.getOutput(access));
        }
        return outputs;
    }
    
    std::vector<const Input *> AddressIndex::getInputs(const Address &address, const ChainAccess &access) const {
        auto pointers = getOutputPointers(address);
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
    
    
    std::vector<Transaction> AddressIndex::getTransactions(const Address &address, const ChainAccess &access) const {
        auto pointers = getOutputPointers(address);
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
    
    std::vector<blocksci::Transaction> AddressIndex::getOutputTransactions(const blocksci::Address &address, const ChainAccess &access) const {
        auto pointers = getOutputPointers(address);
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size());
        for (auto &pointer : pointers) {
            txes.insert(pointer.getTransaction(access));
        }
        return {txes.begin(), txes.end()};
    }
    
    std::vector<Transaction> AddressIndex::getInputTransactions(const Address &address, const ChainAccess &access) const {
        auto pointers = getOutputPointers(address);
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
    
    std::vector<blocksci::OutputPointer> AddressIndex::getOutputPointers(const blocksci::Address &address) const {
        sqlite3_stmt *stmt;
        std::vector<blocksci::OutputPointer> outputs;
        std::stringstream ss;
        ss << "SELECT TX_INDEX, OUTPUT_NUM, ADDRESS_TYPE FROM " << scriptName(scriptType(address.type)) << " WHERE ADDRESS_NUM = ?" ;
        auto rc = sqlite3_prepare_v2(addressDb, ss.str().c_str(), -1, &stmt, 0);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(addressDb));
            return outputs;
        }
        sqlite3_bind_int(stmt, 1, static_cast<int32_t>(address.addressNum));
        
        
        while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            auto txNum = sqlite3_column_int64(stmt, 0);
            auto outputNum = sqlite3_column_int(stmt, 1);
            outputs.push_back({static_cast<uint32_t>(txNum), static_cast<uint16_t>(outputNum)});
        }
        
        sqlite3_finalize(stmt);
        return outputs;
    }
}

