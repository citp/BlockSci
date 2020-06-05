//
//  equiv_address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#ifndef equiv_address_hpp
#define equiv_address_hpp

#include "address_fwd.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/core/core_fwd.hpp>
#include <blocksci/core/typedefs.hpp>

#include <range/v3/view/any_view.hpp>

#include <unordered_set>

namespace blocksci {
    class DataAccess;

    /** Represents all equivalent addresses for a given Address object
     *
     * @see https://citp.github.io/BlockSci/reference/addresses/equiv_address.html
     */
    class BLOCKSCI_EXPORT EquivAddress {
        bool scriptEquivalent;

        DataAccess *access;

        /** Set of equivalent addresses */
        std::unordered_set<Address> addresses;
        
        friend struct std::hash<EquivAddress>;
    public:
        EquivAddress(const Address &address, bool scriptEquivalent_);
        EquivAddress(const DedupAddress &address, bool scriptEquivalent_, DataAccess &access_);
        
        bool operator==(const EquivAddress &other) const {
            if (scriptEquivalent != other.scriptEquivalent) {
                return false;
            }
            return addresses == other.addresses;
        }
        
        std::string toString() const;
        
        size_t size() const {
            return addresses.size();
        }
        
        std::unordered_set<Address>::const_iterator begin() const {
            return addresses.begin();
        }
        
        std::unordered_set<Address>::const_iterator end() const {
            return addresses.end();
        }
        
        bool isScriptEquiv() const {
            return scriptEquivalent;
        }
        
        ranges::any_view<OutputPointer> getOutputPointers() const;
        int64_t calculateBalance(BlockHeight height) const;
        ranges::any_view<Output> getOutputs() const;
        ranges::any_view<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
        
        bool operator==(const EquivAddress& b) {
            return addresses == b.addresses;
        }
        
        bool operator!=(const EquivAddress& b) {
            return addresses != b.addresses;
        }
    };
}

namespace std {
    template <>
    struct BLOCKSCI_EXPORT hash<blocksci::EquivAddress> {
        typedef blocksci::EquivAddress argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &equiv) const;
    };
}

#endif /* equiv_address_hpp */
