//
//  output_spend_data.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#include "output_spend_data.hpp"
#include "script_output.hpp"

#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/address/address_types.hpp>

#include <type_traits>

struct SpendDataGenerator {
    template <blocksci::AddressType::Enum type>
    SpendDataType operator()(const ScriptOutput<type> &output) const {
        return SpendData<type>(output);
    }
};

SpendDataType generateFromScriptAddress(const blocksci::AnyScript &scriptData) {
    return mpark::visit([](auto &script) -> SpendDataType { return SpendData<std::remove_cv_t<std::remove_reference_t<decltype(script)>>::addressType>{script}; }, scriptData.wrapped);
}

AnySpendData::AnySpendData(const AnyScriptOutput &output) : wrapped(mpark::visit(SpendDataGenerator(), output.wrapped)) {}

AnySpendData::AnySpendData(const blocksci::AnyScript &scriptData) : wrapped(generateFromScriptAddress(scriptData)) {}

SpendData<blocksci::AddressType::Enum::MULTISIG>::SpendData(const ScriptOutput<blocksci::AddressType::Enum::MULTISIG> &output) {
    uint32_t i = 0;
    for (auto &address : output.data.addresses) {
        addresses.at(i) = address.data.pubkey;
        i++;
    }
    addressCount = i;
}

SpendData<blocksci::AddressType::Enum::MULTISIG>::SpendData(const blocksci::ScriptAddress<blocksci::AddressType::Enum::MULTISIG> &output) {
    uint32_t i = 0;
    for (auto pubkey : output.pubkeyScripts()) {
        addresses.at(i) = *pubkey.getPubkey();
        i++;
    }
    addressCount = i;
}
