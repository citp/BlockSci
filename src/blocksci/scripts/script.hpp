//
//  script.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef script_hpp
#define script_hpp

#include "script_data.hpp"
#include <blocksci/address/address.hpp>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    class BLOCKSCI_EXPORT ScriptBase : public Address  {
        const ScriptDataBase *rawData;
        
    protected:
        const ScriptDataBase *getData() const {
            return rawData;
        }
    public:
        ScriptBase() = default;
        ScriptBase(uint32_t scriptNum_, AddressType::Enum type_, DataAccess &access_, const ScriptDataBase *rawData_) : Address(scriptNum_, type_, access_), rawData(rawData_) {}
        
        void visitPointers(const std::function<void(const Address &)> &) const {}

        uint32_t getFirstTxIndex() const {
            return rawData->txFirstSeen;
        }
        
        ranges::optional<uint32_t> getTxRevealedIndex() const {
            auto txRevealedIndex = rawData->txFirstSpent;
            if (txRevealedIndex != std::numeric_limits<uint32_t>::max()) {
                return txRevealedIndex;
            } else {
                return ranges::nullopt;
            }
        }

        bool hasBeenSpent() const {
            return getTxRevealedIndex().has_value();
        }
        
        Transaction getFirstTransaction() const;
        ranges::optional<Transaction> getTransactionRevealed() const;
    };
} // namespace blocksci

namespace std {
    template <>
    struct hash<blocksci::ScriptBase> {
        typedef blocksci::ScriptBase argument_type;
        typedef size_t  result_type;
        result_type operator()(const blocksci::ScriptBase &b) const {
            return std::hash<blocksci::Address>{}(b);
        }
    };
}

#endif /* script_hpp */
