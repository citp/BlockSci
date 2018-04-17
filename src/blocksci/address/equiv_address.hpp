//
//  equiv_address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#ifndef equiv_address_hpp
#define equiv_address_hpp

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/address/dedup_address.hpp>
#include <blocksci/util/data_access.hpp>

#include <range/v3/view/join.hpp>

#include <sstream>
#include <unordered_set>

namespace blocksci {
    class EquivAddress {
        std::unordered_set<Address> addresses;
        bool scriptEquivalent;
        DataAccess &access;
        
        friend struct std::hash<EquivAddress>;
        
        EquivAddress(uint32_t scriptNum, EquivAddressType::Enum type, bool scriptEquivalent_, DataAccess &access_);
    public:
        EquivAddress(const Address &address, bool scriptEquivalent_) : EquivAddress(address.scriptNum, equivType(address.type), scriptEquivalent_, address.getAccess()) {}
        EquivAddress(const DedupAddress &address, bool scriptEquivalent_, DataAccess &access_) :
        EquivAddress(address.scriptNum, equivType(address.type), scriptEquivalent_, access_) {}
        
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
        
        auto getOutputPointers() const {
            const auto &access_ = access;
            return addresses
            | ranges::view::transform([&access_](const Address &address) { return access_.addressIndex.getOutputPointers(address); })
            | ranges::view::join
            ;
        }
        
        int64_t calculateBalance(BlockHeight height);
        ranges::any_view<Output> getOutputs();
        std::vector<Input> getInputs();
        std::vector<Transaction> getTransactions();
        std::vector<Transaction> getOutputTransactions();
        std::vector<Transaction> getInputTransactions();
    };
}

namespace std {
    template <>
    struct hash<blocksci::EquivAddress> {
        typedef blocksci::EquivAddress argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &equiv) const {
            std::size_t seed = 123954;
            for (const auto &address : equiv.addresses) {
                seed ^= address.scriptNum + address.type;
            }
            return seed;
        }
    };
}

#endif /* equiv_address_hpp */
