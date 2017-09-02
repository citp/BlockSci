//
//  address_types_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "address_types.hpp"
#include "data_access.hpp"

namespace blocksci {
    using namespace address;
    
    Pubkey::ScriptAddress(uint32_t addressNum) : Pubkey(DataAccess::Instance().scripts, addressNum) {}
    
    std::string Pubkey::addressString() const {
        return addressString(DataAccess::Instance().config);
    }
    
    PubkeyHash::ScriptAddress(uint32_t addressNum) : PubkeyHash(DataAccess::Instance().scripts, addressNum) {}
    std::string PubkeyHash::addressString() const {
        return addressString(DataAccess::Instance().config);
    }
    
    ScriptHash::ScriptAddress(uint32_t addressNum) : ScriptHash(DataAccess::Instance().scripts, addressNum) {}
    std::string ScriptHash::addressString() const {
        return addressString(DataAccess::Instance().config);
    }
    
    Multisig::ScriptAddress(uint32_t addressNum) : Multisig(DataAccess::Instance().scripts, addressNum) {}
    
    Nonstandard::ScriptAddress(uint32_t addressNum) : Nonstandard(DataAccess::Instance().scripts, addressNum) {}
    OpReturn::ScriptAddress(uint32_t addressNum) : OpReturn(DataAccess::Instance().scripts, addressNum) {}
}
