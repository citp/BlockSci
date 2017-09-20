//
//  database.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/20/17.
//
//

#include "database.hpp"

namespace blocksci {
    sqlite3 *openDB(const char *filename);
    
    sqlite3 *openDB(const char *filename) {
        sqlite3 *db;
        auto rc = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);
        if (rc){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
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
