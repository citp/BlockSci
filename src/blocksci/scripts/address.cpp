//
//  address.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address.hpp"

#include "script_info.hpp"
#include "address_types.hpp"

#include <iostream>


namespace blocksci {
    
    template<blocksci::ScriptType::Enum type>
    struct AddressCreateFunctor {
        static std::unique_ptr<Address> f(const ScriptAccess &access, const AddressPointer &pointer) {
            return std::make_unique<ScriptAddress<type>>(access, pointer.addressNum);
        }
    };
    
    std::unique_ptr<Address> Address::create(const ScriptAccess &access, const AddressPointer &pointer) {
        static constexpr auto table = blocksci::make_dynamic_table<AddressCreateFunctor>();
        static constexpr std::size_t size = blocksci::ScriptType::all.size();
        
        auto index = static_cast<size_t>(pointer.type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](access, pointer);
    }

}
