//
//  equiv_address.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#define BLOCKSCI_WITHOUT_SINGLETON


#include "equiv_address.hpp"
#include "address.hpp"
#include "address_info.hpp"
#include "scripts/bitcoin_base58.hpp"
#include "scripts/script_access.hpp"
#include "scripts/scripts_fwd.hpp"
#include "scripts/script_variant.hpp"
#include "chain/transaction.hpp"
#include "chain/output.hpp"
#include "index/address_index.hpp"
#include "index/hash_index.hpp"

#include <unordered_set>

namespace blocksci {
    
    EquivAddress::EquivAddress() : scriptNum(0), type(DedupAddressType::NONSTANDARD) {}
    
    EquivAddress::EquivAddress(uint32_t addressNum_, DedupAddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
    
    std::string EquivAddress::toString() const {
        if (scriptNum == 0) {
            return "InvalidEquivAddress()";
        } else {
            std::stringstream ss;
            ss << "EquivAddress(";
            ss << "scriptNum=" << scriptNum;
            ss << ", type=" << equivAddressName(type);
            ss << ")";
            return ss.str();
        }
    }
    
    uint64_t EquivAddress::calculateBalance(BlockHeight height) const {
        uint64_t value = 0;
        if (height == 0) {
            for (auto &output : access->getOutputs(*this)) {
                if (!output.isSpent()) {
                    value += output.getValue();
                }
            }
        } else {
            for (auto &output : access->getOutputs(*this)) {
                if (output.blockHeight <= height && (!output.isSpent() || output.getSpendingTx()->blockHeight > height)) {
                    value += output.getValue();
                }
            }
        }
        return value;
    }
    
    std::vector<Output> EquivAddress::getOutputs() const {
        return access->getOutputs(*this);
    }
    
    std::vector<Input> EquivAddress::getInputs() const {
        return access->getInputs(*this);
    }
    
    std::vector<Transaction> EquivAddress::getTransactions() const {
        return access->getTransactions(*this);
    }
    
    std::vector<Transaction> EquivAddress::getOutputTransactions() const {
        return access->getOutputTransactions(*this);
    }
    
    std::vector<Transaction> EquivAddress::getInputTransactions() const {
        return access->getInputTransactions(*this);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::EquivAddress &address) {
    os << address.toString();
    return os;
}
