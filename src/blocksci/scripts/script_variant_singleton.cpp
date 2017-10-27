//
//  script_variant_singleton.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/23/17.
//

#include "script_variant.hpp"
#include "scripts.hpp"
#include "data_access.hpp"

#include <boost/variant.hpp>

namespace blocksci {
    std::string AnyScript::toPrettyString() const {
        auto &instance = DataAccess::Instance();
        return toPrettyString(instance.config, *instance.scripts);
    }

}
