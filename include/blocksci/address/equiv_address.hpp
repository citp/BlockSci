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
    
    class BLOCKSCI_EXPORT EquivAddress {
        std::unordered_set<Address> addresses;
        bool scriptEquivalent;
        DataAccess &access;
        
        friend struct std::hash<EquivAddress>;
        
        EquivAddress(uint32_t scriptNum, EquivAddressType::Enum type, bool scriptEquivalent_, DataAccess &access_);
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
        
        ranges::any_view<OutputPointer> getOutputPointers();
        int64_t calculateBalance(BlockHeight height);
        ranges::any_view<Output> getOutputs();
        std::vector<Input> getInputs();
        std::vector<Transaction> getTransactions();
        std::vector<Transaction> getOutputTransactions();
        std::vector<Transaction> getInputTransactions();
        
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
