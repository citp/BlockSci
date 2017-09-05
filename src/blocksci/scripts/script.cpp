//
//  script.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script.hpp"

#include <blocksci/address/address_info.hpp>
#include "pubkey_script.hpp"
#include "pubkeyhash_script.hpp"
#include "multisig_script.hpp"
#include "scripthash_script.hpp"
#include "nulldata_script.hpp"
#include "nonstandard_script.hpp"

#include <iostream>


namespace blocksci {
    
    template<blocksci::AddressType::Enum type>
    struct ScriptCreateFunctor {
        static std::unique_ptr<Script> f(const ScriptAccess &access, const Address &address) {
            return std::make_unique<ScriptAddress<type>>(access, address.addressNum);
        }
    };
    
    std::unique_ptr<Script> Script::create(const ScriptAccess &access, const Address &address) {
        static constexpr auto table = blocksci::make_dynamic_table<ScriptCreateFunctor>();
        static constexpr std::size_t size = blocksci::AddressType::all.size();
        
        auto index = static_cast<size_t>(address.type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](access, address);
    }

}
