//
//  access.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blocksci_access_hpp
#define blocksci_access_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/address/address.hpp>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;

    /** Wrapper class that manages all data access for a blockchain */
    class BLOCKSCI_EXPORT Access {
        DataAccess *access;
    public:
        Access(DataAccess *access_) : access(access_) {}

        Transaction txWithIndex(uint32_t index) const {
            return Transaction{index, *access};
        }

        Transaction txWithHash(const std::string &hash) const {
            return Transaction{hash, *access};
        }

        Address addressFromIndex(uint32_t index, AddressType::Enum type) const {
            return Address{index, type, *access};
        }

        ranges::optional<Address> addressFromString(const std::string &addressString) const {
            return getAddressFromString(addressString, *access);
        }

        std::vector<Address> addressesWithPrefix(const std::string &prefix) const {
            return getAddressesWithPrefix(prefix, *access);
        }
    };
    
} // namespace blocksci


#endif /* blocksci_access_hpp */
