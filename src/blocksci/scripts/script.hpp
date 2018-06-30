//
//  script.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef script_hpp
#define script_hpp

#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address.hpp>

#include <vector>
#include <memory>
#include <stdio.h>

namespace blocksci {
    
    template <AddressType::Enum AddressType>
    class ScriptAddress;
    
    struct Script {
        virtual std::string toString(const DataConfiguration &config) const = 0;
        virtual std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const = 0;
        
        virtual ~Script() = default;
        
        virtual std::vector<Address> nestedAddresses() const {
            return std::vector<Address>();
        }
        
        virtual bool operator==(const Script &other) = 0;
        
        static std::unique_ptr<Script> create(const ScriptAccess &access, const Address &address);
        
        // requires DataAccess
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        static std::unique_ptr<Script> create(const Address &address);
        std::string toString() const;
        std::string toPrettyString() const;
        #endif
        
    };
    
}
#ifndef BLOCKSCI_WITHOUT_SINGLETON
std::ostream &operator<<(std::ostream &os, const blocksci::Script &script);
#endif

#endif /* script_hpp */
