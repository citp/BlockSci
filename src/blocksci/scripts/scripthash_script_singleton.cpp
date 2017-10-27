//
//  scripthash_script_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "scripthash_script.hpp"
#include "data_access.hpp"
#include "chain/transaction.hpp"
#include "scripts.hpp"
#include "script_variant.hpp"

#include <boost/optional/optional.hpp>
#include <boost/variant.hpp>

namespace blocksci {
    using namespace script;
    ScriptHash::ScriptAddress(uint32_t addressNum) : ScriptHash(*DataAccess::Instance().scripts, addressNum) {}
    std::string ScriptHash::addressString() const {
        return addressString(DataAccess::Instance().config);
    }
    
    AnyScript ScriptHash::wrappedScript() const {
        return wrappedScript(*DataAccess::Instance().scripts);
    }
    
    boost::optional<Transaction> ScriptHash::transactionRevealed() const {
        return transactionRevealed(*DataAccess::Instance().chain);
    }
    
    std::string ScriptHash::toString() const {
        return toString(DataAccess::Instance().config);
    }
    
    std::string ScriptHash::toPrettyString() const {
        return toPrettyString(DataAccess::Instance().config, *DataAccess::Instance().scripts);
    }
}
