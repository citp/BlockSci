//
//  script.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "util/util.hpp"
#include "script.hpp"
#include "script_data.hpp"
#include "script_info.hpp"

#include "script_variant.hpp"

#include "address/address.hpp"
#include "address/address_info.hpp"
#include "index/address_index.hpp"

#include "chain/transaction.hpp"
#include "chain/output.hpp"

#include <iostream>


namespace blocksci {
    
    Script::Script(uint32_t scriptNum_, AddressType::Enum type_, const ScriptDataBase &data, const ScriptAccess &scripts_) : Address(scriptNum_, type_), access(&scripts_), firstTxIndex(data.txFirstSeen), txRevealed(data.txFirstSpent) {}
    
    Transaction Script::getFirstTransaction(const ChainAccess &chain) const {
        return Transaction(firstTxIndex, chain);
    }
    
    ranges::optional<Transaction> Script::getTransactionRevealed(const ChainAccess &chain) const {
        if (hasBeenSpent()) {
            return Transaction(txRevealed, chain);
        } else {
            return ranges::nullopt;
        }
    }
    
//    std::string Script::toString() const {
//        if (scriptNum == 0) {
//            return "InvalidScript()";
//        } else {
//            std::stringstream ss;
//            ss << "Script(";
//            ss << "scriptNum=" << scriptNum;
//            ss << ", type=" << scriptName(type);
//            ss << ")";
//            return ss.str();
//        }
//    }
//
//    AnyScript Script::getScript(const ScriptAccess &scripts) const {
//        return AnyScript(*this, scripts);
//    }
//
//    uint64_t Script::calculateBalance(BlockHeight height, const AddressIndex &index, const ChainAccess &chain) const {
//        uint64_t value = 0;
//        if (height == 0) {
//            for (auto &output : index.getOutputs(*this, chain)) {
//                if (!output.isSpent()) {
//                    value += output.getValue();
//                }
//            }
//        } else {
//            for (auto &output : index.getOutputs(*this, chain)) {
//                if (output.blockHeight <= height && (!output.isSpent() || output.getSpendingTx()->blockHeight > height)) {
//                    value += output.getValue();
//                }
//            }
//        }
//        return value;
//    }
//
//    std::vector<Output> Script::getOutputs(const AddressIndex &index, const ChainAccess &chain) const {
//        return index.getOutputs(*this, chain);
//    }
//
//    std::vector<Input> Script::getInputs(const AddressIndex &index, const ChainAccess &chain) const {
//        return index.getInputs(*this, chain);
//    }
//
//    std::vector<Transaction> Script::getTransactions(const AddressIndex &index, const ChainAccess &chain) const {
//        return index.getTransactions(*this, chain);
//    }
//
//    std::vector<Transaction> Script::getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
//        return index.getOutputTransactions(*this, chain);
//    }
//
//    std::vector<Transaction> Script::getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
//        return index.getInputTransactions(*this, chain);
//    }

}
