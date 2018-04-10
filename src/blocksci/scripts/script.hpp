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
#include "script_data.hpp"

#include <blocksci/address/address_fwd.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/transaction.hpp>

#include <range/v3/utility/optional.hpp>

#include <functional>
#include <limits>
#include <memory>
#include <vector>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    template<typename T>
    class ScriptBase : public Address  {
    public:
        ScriptBase(uint32_t scriptNum_, AddressType::Enum type_, DataAccess &access_) : Address(scriptNum_, type_, access_) {}
        
        void visitPointers(const std::function<void(const Address &)> &) const {}

        uint32_t getFirstTxIndex() const {
            return static_cast<const T *>(this)->rawData->txFirstSeen;
        }
        uint32_t getTxRevealedIndex() const {
            return static_cast<const T *>(this)->rawData->txFirstSpent;
        }
        
        Transaction getFirstTransaction() const {
            return Transaction(getFirstTxIndex(), getAccess());
        }
        
        ranges::optional<Transaction> getTransactionRevealed() const {
            if (hasBeenSpent()) {
                return Transaction(getTxRevealedIndex(), getAccess());
            } else {
                return ranges::nullopt;
            }
        }

        bool hasBeenSpent() const {
            return getTxRevealedIndex() != std::numeric_limits<uint32_t>::max();
        }
    };
} // namespace blocksci

#endif /* script_hpp */
