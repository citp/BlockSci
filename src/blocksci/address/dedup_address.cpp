//
//  dedup_address.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#define BLOCKSCI_WITHOUT_SINGLETON


#include "dedup_address.hpp"
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
    
    DedupAddress::DedupAddress() : scriptNum(0), type(DedupAddressType::NONSTANDARD) {}
    
    DedupAddress::DedupAddress(uint32_t addressNum_, DedupAddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
    
    std::string DedupAddress::toString() const {
        if (scriptNum == 0) {
            return "InvalidDedupAddress()";
        } else {
            std::stringstream ss;
            ss << "DedupAddress(";
            ss << "scriptNum=" << scriptNum;
            ss << ", type=" << dedupAddressName(type);
            ss << ")";
            return ss.str();
        }
    }
    
    uint64_t DedupAddress::calculateBalance(BlockHeight height, const AddressIndex &index, const ChainAccess &chain) const {
        uint64_t value = 0;
        if (height == 0) {
            for (auto &output : index.getOutputs(*this, chain)) {
                if (!output.isSpent()) {
                    value += output.getValue();
                }
            }
        } else {
            for (auto &output : index.getOutputs(*this, chain)) {
                if (output.blockHeight <= height && (!output.isSpent() || output.getSpendingTx()->blockHeight > height)) {
                    value += output.getValue();
                }
            }
        }
        return value;
    }
    
    std::vector<Output> DedupAddress::getOutputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputs(*this, chain);
    }
    
    std::vector<Input> DedupAddress::getInputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputs(*this, chain);
    }
    
    std::vector<Transaction> DedupAddress::getTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getTransactions(*this, chain);
    }
    
    std::vector<Transaction> DedupAddress::getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputTransactions(*this, chain);
    }
    
    std::vector<Transaction> DedupAddress::getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputTransactions(*this, chain);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::DedupAddress &address) {
    os << address.toString();
    return os;
}
