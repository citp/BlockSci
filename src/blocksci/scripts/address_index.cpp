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
#include "address_pointer.hpp"
#include "data_configuration.hpp"

#include <vector>
#include <sstream>
#include <iostream>

namespace blocksci {
    
    const std::string AddressIndex::addrTables[] = {"SINGLE_ADDRESSES", "MULTISIG_ADDRESSES", "P2SH_ADDRESSES"};
    
    AddressIndex::AddressIndex(const DataConfiguration &config) {
        auto rc = sqlite3_open(config.addressDBFilePath().c_str(), &addressDb);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addressDb));
        }
    }
    
    AddressIndex::AddressIndex(const AddressIndex &other) {
        
        auto rc = sqlite3_open(sqlite3_db_filename(other.addressDb, "main"), &addressDb);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addressDb));
        }
    }
    
    AddressIndex::~AddressIndex() {
        sqlite3_close(addressDb);
    }
    
    
    AddressIndex& AddressIndex::operator=(const AddressIndex& other) {
        sqlite3_close(addressDb);
        auto rc = sqlite3_open(sqlite3_db_filename(other.addressDb, "main"), &addressDb);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addressDb));
        }
        return *this;
    }
    
    std::vector<const blocksci::Output *> AddressIndex::getOutputs(const ChainAccess &access, const blocksci::AddressPointer &address) const {
        auto pointers = getOutputPointers(address);
        std::vector<const Output *> outputs;
        outputs.reserve(pointers.size());
        for (auto &pointer : pointers) {
            outputs.push_back(&pointer.getOutput(access));
        }
        return outputs;
    }
    
    std::vector<blocksci::Transaction> AddressIndex::getOutputTransactions(const ChainAccess &access, const blocksci::AddressPointer &address) const {
        auto pointers = getOutputPointers(address);
        std::vector<blocksci::Transaction> outputs;
        outputs.reserve(pointers.size());
        for (auto &pointer : pointers) {
            outputs.push_back(pointer.getTransaction(access));
        }
        return outputs;
    }
    
    std::vector<blocksci::OutputPointer> AddressIndex::getOutputPointers(const blocksci::AddressPointer &address) const {
        sqlite3_stmt *stmt;
        std::vector<blocksci::OutputPointer> outputs;
        std::stringstream ss;
        ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << addrTables[address.getDBType()] << " WHERE ADDRESS_NUM = ?" ;
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
    
//    std::vector<OutputPointer> AddressIndex::getOutputPointers(const std::vector<AddressPointer> &addresses) const {
//        sqlite3_stmt *stmt;
//        std::vector<blocksci::OutputPointer> outputs;
//        std::stringstream ss;
//        ss << "SELECT TX_INDEX, OUTPUT_NUM FROM " << addrTables[address.getDBType()] << " WHERE ADDRESS_NUM = ?" ;
//        auto rc = sqlite3_prepare_v2(addressDb, ss.str().c_str(), -1, &stmt, 0);
//        if( rc != SQLITE_OK ){
//            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(addressDb));
//            return outputs;
//        }
//        sqlite3_bind_int(stmt, 1, static_cast<int32_t>(address.addressNum));
//        
//        
//        while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//            auto txNum = sqlite3_column_int64(stmt, 0);
//            auto outputNum = sqlite3_column_int(stmt, 1);
//            outputs.push_back({static_cast<uint32_t>(txNum), static_cast<uint16_t>(outputNum)});
//        }
//        
//        sqlite3_finalize(stmt);
//        return outputs;
//    }
}

