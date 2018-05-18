//
//  nulldata_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/nulldata_script.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>

#include <sstream>
#include <cstring>

namespace blocksci {

    ScriptAddress<AddressType::NULL_DATA>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}

    std::string ScriptAddress<AddressType::NULL_DATA>::toPrettyString() const {
        std::stringstream ss;
        ss << "NulldataAddressData(" << getData() << ")";
        return ss.str();
    }

    bool ScriptAddress<AddressType::NULL_DATA>::isSegwitMarker() const {
        auto data = getData();
        uint32_t startVal;
        std::memcpy(&startVal, data.c_str(), sizeof(startVal));
        return startVal == 0xaa21a9ed;
    }
} // namespace blocksci
