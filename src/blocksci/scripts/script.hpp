//
//  script.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef script_hpp
#define script_hpp

#include "scripts_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/address/address.hpp>

#include <range/v3/utility/optional.hpp>

#include <functional>
#include <vector>
#include <memory>
#include <limits>

namespace blocksci {
    
    struct DataConfiguration;
    class DataAccess;
    
    struct Script : public Address {
        const DataAccess *access;
        uint32_t firstTxIndex;
        uint32_t txRevealed;
        
        Script(uint32_t scriptNum_, AddressType::Enum type_, const ScriptDataBase &data, const DataAccess &access);
        Script(const Address &address, const ScriptDataBase &data, const DataAccess &access);
        
        void visitPointers(const std::function<void(const Address &)> &) const {}
        
        Transaction getFirstTransaction() const;
        ranges::optional<Transaction> getTransactionRevealed() const;

        bool hasBeenSpent() const {
            return txRevealed != std::numeric_limits<uint32_t>::max();
        }
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
