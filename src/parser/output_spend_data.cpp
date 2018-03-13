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

struct SpendDataGenerator {
    template <blocksci::AddressType::Enum type>
    SpendDataType operator()(const ScriptOutput<type> &output) const {
        return SpendData<type>(output);
    }
};

template<blocksci::AddressType::Enum type>
struct ScriptAddressGenerator {
    static SpendDataType f(const blocksci::AnyScript &script) {
        return SpendData<type>(mpark::get<blocksci::ScriptAddress<type>>(script.wrapped));
    }
};

static auto scriptAddressGeneratorTable = blocksci::make_dynamic_table<blocksci::AddressType, ScriptAddressGenerator>();

SpendDataType generateFromScriptAddress(const blocksci::AnyScript &scriptData, blocksci::AddressType::Enum type) {
    auto index = static_cast<size_t>(type);
    if (index >= blocksci::AddressType::size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return scriptAddressGeneratorTable[index](scriptData);
}

AnySpendData::AnySpendData(const AnyScriptOutput &output) : wrapped(mpark::visit(SpendDataGenerator(), output.wrapped)) {}

AnySpendData::AnySpendData(const blocksci::AnyScript &scriptData, blocksci::AddressType::Enum addressType) : wrapped(generateFromScriptAddress(scriptData, addressType)) {}

SpendData<blocksci::AddressType::Enum::MULTISIG>::SpendData(const ScriptOutput<blocksci::AddressType::Enum::MULTISIG> &output) {
    uint32_t i = 0;
    for (auto &address : output.data.addresses) {
        addresses[i] = address.data.pubkey;
        i++;
    }
    addressCount = i;
}

SpendData<blocksci::AddressType::Enum::MULTISIG>::SpendData(const blocksci::ScriptAddress<blocksci::AddressType::Enum::MULTISIG> &output) {
    uint32_t i = 0;
    for (auto pubkey : output.pubkeyScripts()) {
        addresses[i] = *pubkey.getPubkey();
        i++;
    }
    addressCount = i;
}
