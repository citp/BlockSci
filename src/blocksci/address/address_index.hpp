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
#include <blocksci/scripts/script_type.hpp>
#include <blocksci/chain/chain_fwd.hpp>

#include <lmdbxx/lmdb++.h>

#include <unordered_map>
#include <string>
#include <vector>

namespace blocksci {
    
    struct DataConfiguration;
    struct Address;
    struct Script;
    class ChainAccess;
    
    lmdb::env createAddressIndexEnviroment(const std::string &path);
    
    class AddressIndex {
        lmdb::env env;
        std::unordered_map<blocksci::ScriptType::Enum,  lmdb::dbi> scriptDbs;
    public:
        
        AddressIndex(const DataConfiguration &config);
        
        std::vector<OutputPointer> getOutputPointers(const Address &address) const;
        std::vector<OutputPointer> getOutputPointers(const Script &script) const;
        
        std::vector<Output> getOutputs(const Address &address, const ChainAccess &access) const;
        std::vector<Input> getInputs(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getTransactions(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getOutputTransactions(const Address &address, const ChainAccess &access) const;
        std::vector<Transaction> getInputTransactions(const Address &address, const ChainAccess &access) const;
        
        std::vector<Output> getOutputs(const Script &script, const ChainAccess &access) const;
        std::vector<Input> getInputs(const Script &script, const ChainAccess &access) const;
        std::vector<Transaction> getTransactions(const Script &script, const ChainAccess &access) const;
        std::vector<Transaction> getOutputTransactions(const Script &script, const ChainAccess &access) const;
        std::vector<Transaction> getInputTransactions(const Script &script, const ChainAccess &access) const;
    };
}


#endif /* address_index_hpp */
