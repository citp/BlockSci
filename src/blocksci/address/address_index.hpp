//
//  address_index.hpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#ifndef address_index_hpp
#define address_index_hpp

#include <blocksci/address/address_types.hpp>
#include <blocksci/scripts/script_info.hpp>

#include <SQLiteCpp/Database.h>

#include <unordered_set>
#include <string>
#include <vector>

namespace blocksci {
    
    struct DataConfiguration;
    struct OutputPointer;
    struct Output;
    struct Input;
    struct Transaction;
    struct Address;
    struct Script;
    class ChainAccess;
    
    class AddressIndex {
        SQLite::Database db;
        mutable std::array<SQLite::Statement, AddressType::size> addressQueries;
        mutable std::array<SQLite::Statement, ScriptType::size> scriptQueries;
        
    public:
        
        AddressIndex(const DataConfiguration &config);
        
        std::vector<OutputPointer> getOutputPointers(const Address &address) const;
        std::vector<OutputPointer> getOutputPointers(const Script &script) const;
        
        std::vector<const Output *> getOutputs(const Address &address, const ChainAccess &access) const;
        std::vector<const Input *> getInputs(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getTransactions(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getOutputTransactions(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getInputTransactions(const Address &address, const ChainAccess &access) const;
        
        std::vector<const Output *> getOutputs(const Script &script, const ChainAccess &access) const;
        std::vector<const Input *> getInputs(const Script &script, const ChainAccess &access) const;
        std::vector<Transaction> getTransactions(const Script &script, const ChainAccess &access) const;
        std::vector<Transaction> getOutputTransactions(const Script &script, const ChainAccess &access) const;
        std::vector<Transaction> getInputTransactions(const Script &script, const ChainAccess &access) const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        std::vector<const Output *> getOutputs(const Address &address) const;
        std::vector<const Input *> getInputs(const Address &address) const;
        std::vector<Transaction> getTransactions(const Address &address) const;
        std::vector<Transaction> getOutputTransactions(const Address &address) const;
        std::vector<Transaction> getInputTransactions(const Address &address) const;
        
        std::vector<const Output *> getOutputs(const Script &script) const;
        std::vector<const Input *> getInputs(const Script &script) const;
        std::vector<Transaction> getTransactions(const Script &script) const;
        std::vector<Transaction> getOutputTransactions(const Script &script) const;
        std::vector<Transaction> getInputTransactions(const Script &script) const;
        #endif
    };
}


#endif /* address_index_hpp */
