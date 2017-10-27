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

#include <functional>
#include <vector>
#include <memory>

namespace blocksci {
    
    struct Address;
    struct DataConfiguration;
    class ScriptAccess;
    class ChainAccess;
    class AddressIndex;
    struct Output;
    struct Input;
    struct Transaction;
    
    struct Script {
        uint32_t scriptNum;
        ScriptType::Enum type;
        
        Script(uint32_t scriptNum_, ScriptType::Enum type_) : scriptNum(scriptNum_), type(type_) {}
        
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
