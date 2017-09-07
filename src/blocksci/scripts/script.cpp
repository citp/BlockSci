//
//  script.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "util.hpp"
#include "script.hpp"

#include <blocksci/address/address_info.hpp>
#include "pubkey_script.hpp"
#include "multisig_script.hpp"
#include "scripthash_script.hpp"
#include "nulldata_script.hpp"
#include "nonstandard_script.hpp"

#include <iostream>


namespace blocksci {
    
    template<AddressType::Enum type>
    struct ScriptCreateFunctor {
        static std::unique_ptr<Script> f(const ScriptAccess &access, const Address &address) {
            constexpr auto scriptType = AddressInfo<type>::scriptType;
            return std::make_unique<ScriptAddress<scriptType>>(access, address.addressNum);
        }
    };
    
    std::unique_ptr<Script> Script::create(const ScriptAccess &access, const Address &address) {
        static constexpr auto table = make_dynamic_table<AddressType, ScriptCreateFunctor>();
        static constexpr std::size_t size = AddressType::all.size();
        
        auto index = static_cast<size_t>(address.type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](access, address);
    }

}
