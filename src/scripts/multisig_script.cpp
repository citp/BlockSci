//
//  multisig_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/multisig_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>

#include <range/v3/view/transform.hpp>

#include <sstream>

namespace blocksci {
    ScriptAddress<AddressType::MULTISIG>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string ScriptAddress<AddressType::MULTISIG>::toString() const {
        std::stringstream ss;
        ss << "MultisigAddress(" << static_cast<int>(getRequired()) << " of " << static_cast<int>(getTotal()) << ")";
        return ss.str();
    }
    
    ranges::any_view<Address, ranges::category::random_access | ranges::category::sized> ScriptAddress<AddressType::MULTISIG>::getAddresses() const {
        auto access_ = &getAccess();
        auto data = getData();
        return data->addresses | ranges::views::transform([access_](uint32_t scriptNum) -> Address {
            return {scriptNum, AddressType::Enum::MULTISIG_PUBKEY, *access_};
        });
    }
    
    ranges::any_view<script::MultisigPubkey, ranges::category::random_access | ranges::category::sized> ScriptAddress<AddressType::MULTISIG>::pubkeyScripts() const {
        return getAddresses() | ranges::views::transform([](Address address) -> script::MultisigPubkey {
            return {address.scriptNum, address.getAccess()};
        });
    }
    
    std::string ScriptAddress<AddressType::MULTISIG>::toPrettyString() const {
            std::stringstream ss;
            ss << "MultisigAddress(" << static_cast<int>(getRequired()) << " of " << static_cast<int>(getTotal()) << " multisig with addresses ";
            uint8_t i = 0;
            RANGES_FOR(auto address, getAddresses()) {
                script::MultisigPubkey pubkeyScript(address.scriptNum, getAccess());
                ss << pubkeyScript.toPrettyString();
                if (i < getTotal() - 1) {
                    ss << ", ";
                }
                i++;
            }
            ss << ")";
            return ss.str();
        }
} // namespace blocksci
