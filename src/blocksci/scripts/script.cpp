//
//  script.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "util.hpp"
#include "script.hpp"
#include "script_data.hpp"

#include "pubkey_script.hpp"
#include "multisig_script.hpp"
#include "scripthash_script.hpp"
#include "nulldata_script.hpp"
#include "nonstandard_script.hpp"

#include "address/address.hpp"
#include "address/address_info.hpp"
#include "address/address_index.hpp"

#include "chain/transaction.hpp"

#include <iostream>


namespace blocksci {
    
    Script::Script(const Address &address) : Script(address.scriptNum, scriptType(address.type)) {}
    
    BaseScript::BaseScript(uint32_t scriptNum_, ScriptType::Enum type_, const ScriptDataBase &data) : Script(scriptNum_, type_), firstTxIndex(data.txFirstSeen), txRevealed(data.txFirstSpent) {}
    
    BaseScript::BaseScript(const Address &address, const ScriptDataBase &data) : BaseScript(address.scriptNum, scriptType(address.type), data) {}
    
    
    Transaction BaseScript::getFirstTransaction(const ChainAccess &chain) const {
        return Transaction::txWithIndex(chain, firstTxIndex);
    }
    
    std::string Script::toString() const {
        if (scriptNum == 0) {
            return "InvalidScript()";
        } else {
            std::stringstream ss;
            ss << "Script(";
            ss << "scriptNum=" << scriptNum;
            ss << ", type=" << scriptName(type);
            ss << ")";
            return ss.str();
        }
    }
    
    std::vector<const Output *> Script::getOutputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputs(*this, chain);
    }
    
    std::vector<const Input *> Script::getInputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputs(*this, chain);
    }
    
    std::vector<Transaction> Script::getTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getTransactions(*this, chain);
    }
    
    std::vector<Transaction> Script::getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputTransactions(*this, chain);
    }
    
    std::vector<Transaction> Script::getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputTransactions(*this, chain);
    }

}
