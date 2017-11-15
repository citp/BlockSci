//
//  script.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef script_hpp
#define script_hpp

#include "script_type.hpp"
#include "scriptsfwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

#include <functional>
#include <vector>
#include <memory>

namespace blocksci {
    
    struct Address;
    struct DataConfiguration;
    class ScriptAccess;
    class AddressIndex;
    
    struct Script {
        uint32_t scriptNum;
        ScriptType::Enum type;
        
        Script() = default;
        Script(uint32_t scriptNum_, ScriptType::Enum type_) : scriptNum(scriptNum_), type(type_) {}
        Script(const Address &address);
        
        bool operator==(const Script& other) const {
            return type == other.type && scriptNum == other.scriptNum;
        }
        
        void visitPointers(const std::function<void(const Address &)> &) const {}

        std::vector<const Output *> getOutputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<const Input *> getInputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        
        std::string toString() const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        std::vector<const Output *> getOutputs() const;
        std::vector<const Input *> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
        #endif
    };
    
    struct BaseScript : public Script {
        uint32_t firstTxIndex;
        uint32_t txRevealed;
        
        BaseScript(uint32_t scriptNum_, ScriptType::Enum type_, const ScriptDataBase &data);
        BaseScript(const Address &address, const ScriptDataBase &data);
        
        Transaction getFirstTransaction(const ChainAccess &chain) const;

        bool hasBeenSpent() const {
            return txRevealed != std::numeric_limits<uint32_t>::max();
        }
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction getFirstTransaction() const;
        #endif
    };
}

namespace std {
    template <>
    struct hash<blocksci::Script> {
        typedef blocksci::Script argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.scriptNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}


#endif /* script_hpp */
