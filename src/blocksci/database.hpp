//
//  database.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/20/17.
//
//

#ifndef database_hpp
#define database_hpp

#include <sqlite3.h>
#include <stdio.h>

namespace blocksci {
    class Database {
    protected:
        sqlite3 *db;
    public:
        
        Database(const char *filename);
        Database(const Database &other);
        Database& operator=(const Database& other);
        
        ~Database();
    };
}


#endif /* database_hpp */
