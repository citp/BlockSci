//
//  address_index.hpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#ifndef address_index_hpp
#define address_index_hpp

#include <sqlite3.h>

#include <string>
#include <vector>
#include <stdio.h>

namespace blocksci {
    
    struct DataConfiguration;
    struct OutputPointer;
    struct Output;
    struct Transaction;
    struct AddressPointer;
    class ChainAccess;
    
    
    
    class AddressIndex {
        sqlite3 *addressDb;
    public:
        const static std::string addrTables[3];
        
        AddressIndex(const DataConfiguration &config);
        AddressIndex(const AddressIndex &other);
        AddressIndex& operator=(const AddressIndex& other);
        ~AddressIndex();
        
        std::vector<OutputPointer> getOutputPointers(const AddressPointer &address) const;
//        std::vector<OutputPointer> getOutputPointers(const std::vector<AddressPointer> &addresses) const;
        std::vector<const Output *> getOutputs(const ChainAccess &access, const AddressPointer &address) const;
        std::vector<Transaction> getOutputTransactions(const ChainAccess &access, const AddressPointer &address) const;
        std::vector<const Output *> getOutputs(const ChainAccess &access, const std::vector<AddressPointer> &addresses) const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        std::vector<const Output *> getOutputs(const AddressPointer &address) const;
        std::vector<Transaction> getOutputTransactions(const AddressPointer &address) const;
        std::vector<const Output *> getOutputs(const std::vector<AddressPointer> &addresses) const;
        #endif
    };
}


#endif /* address_index_hpp */
