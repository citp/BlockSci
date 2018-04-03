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
#include <blocksci/scripts/bitcoin_base58.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

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
}

std::ostream &operator<<(std::ostream &os, const blocksci::DedupAddress &address) {
    os << address.toString();
    return os;
}
