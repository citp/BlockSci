//
//  database.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/20/17.
//
//

#include "database.hpp"

#include <sqlite3.h>

#include <iostream>

namespace blocksci {
    sqlite3 *openDB(const char *filename);
    
    sqlite3 *openDB(const char *filename) {
        sqlite3 *db;
        auto rc = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, nullptr);
        if (rc){
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        }
        return db;
    }
    
    Database::Database(const char *filename) : db(openDB(filename)) {}
    
    Database::Database(const Database &other) : Database(sqlite3_db_filename(other.db, "main")) {}
    
    Database::~Database() {
        sqlite3_close(db);
    }
    
    Database& Database::operator=(const Database& other) {
        sqlite3_close(db);
        db = openDB(sqlite3_db_filename(other.db, "main"));
        return *this;
    }
}
