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
#include <blocksci/address/address.hpp>

#include <vector>
#include <memory>
#include <stdio.h>

namespace blocksci {
    
    struct Script {
        uint32_t scriptNum;
        
        Script(uint32_t scriptNum_) : scriptNum(scriptNum_) {}
        
        virtual ScriptType::Enum type() const = 0;
        
        virtual std::string toString(const DataConfiguration &config) const = 0;
        virtual std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const = 0;
        
        virtual ~Script() = default;
        
        virtual bool operator==(const Script &other) = 0;
        
        static std::unique_ptr<Script> create(const ScriptAccess &access, const Address &address);
        
        virtual void visitPointers(const std::function<void(const Address &)> &) const {}
        
        std::vector<const Output *> getOutputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<const Input *> getInputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        
        // requires DataAccess
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        static std::unique_ptr<Script> create(const Address &address);
        std::string toString() const;
        std::string toPrettyString() const;
        
        std::vector<const Output *> getOutputs() const;
        std::vector<const Input *> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
        #endif
        
    };
    
}
#ifndef BLOCKSCI_WITHOUT_SINGLETON
std::ostream &operator<<(std::ostream &os, const blocksci::Script &script);
#endif

#endif /* script_hpp */
