//
//  address_index.hpp
//  blocksci_address_index
//
//  Created by Harry Kalodner on 7/9/17.
//
//

#ifndef address_index_hpp
#define address_index_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/core/core_fwd.hpp>

#include <range/v3/view/any_view.hpp>

#include <cstring>
#include <unordered_set>
#include <string>
#include <vector>

namespace blocksci {
    class DataAccess;
    class EquivAddress;
    class AddressOutputRange;
    struct State;
    class AddressIndexPriv;
    
    class BLOCKSCI_EXPORT AddressIndex {
        friend class AddressOutputRange;
        
        std::unique_ptr<AddressIndexPriv> impl;
        
        std::unordered_set<Address> getPossibleNestedEquivalentUp(const Address &address) const;
        std::unordered_set<Address> getPossibleNestedEquivalentDown(const Address &address) const;
    public:
        
        AddressIndex(const std::string &path, bool readonly);
        ~AddressIndex();

        bool checkIfExists(const Address &address) const;
        bool checkIfTopLevel(const Address &address) const;
        
        ranges::any_view<OutputPointer> getOutputPointers(const Address &address) const;
        
        std::vector<Address> getPossibleNestedEquivalent(const Address &address) const;
        std::vector<Address> getIncludingMultisigs(const Address &searchAddress) const;
        
        void addNestedAddresses(std::vector<std::pair<blocksci::RawAddress, blocksci::DedupAddress>> nestedCache);
        void addOutputAddresses(std::vector<std::pair<blocksci::RawAddress, blocksci::OutputPointer>> outputCache);
        
        void compactDB();
        
        void rollback(const State &state);
    };
}


#endif /* address_index_hpp */
