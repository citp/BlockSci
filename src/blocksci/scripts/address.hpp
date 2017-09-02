//
//  address.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef address_hpp
#define address_hpp

#include "script_types.hpp"
#include "address_pointer.hpp"

#include <vector>
#include <memory>
#include <stdio.h>

namespace blocksci {
    
    template <ScriptType::Enum ScriptType>
    class ScriptAddress;
    
    struct Address {
        virtual std::string toString(const DataConfiguration &config) const = 0;
        virtual std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const = 0;
        
        virtual ~Address() = default;
        
        virtual std::vector<AddressPointer> nestedAddressPointers() const {
            return std::vector<AddressPointer>();
        }
        
        virtual bool operator==(const Address &other) = 0;
        
        static std::unique_ptr<Address> create(const ScriptAccess &access, const AddressPointer &pointer);
        
        // requires DataAccess
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        static std::unique_ptr<Address> create(const AddressPointer &pointer);
        std::string toString() const;
        std::string toPrettyString() const;
        #endif
        
    };
    
}
#ifndef BLOCKSCI_WITHOUT_SINGLETON
std::ostream &operator<<(std::ostream &os, const blocksci::Address &address);
#endif

#endif /* address_hpp */
