//
//  output_spend_data.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#include "output_spend_data.hpp"
#include "script_output.hpp"

#include <blocksci/scripts/script_variant.hpp>

#include <boost/variant.hpp>

struct SpendDataGenerator : public boost::static_visitor<SpendDataType> {
    template <blocksci::AddressType::Enum type>
    SpendDataType operator()(const ScriptOutput<type> &output) const {
        return SpendData<type>(output);
    }
};

template<blocksci::AddressType::Enum type>
struct ScriptAddressGenerator {
    static SpendDataType f(const blocksci::AnyScript &script, const blocksci::ScriptAccess &scripts) {
        return SpendData<type>(boost::get<blocksci::ScriptAddress<blocksci::scriptType(type)>>(script.wrapped), scripts);
    }
};

static auto scriptAddressGeneratorTable = blocksci::make_dynamic_table<blocksci::AddressType, ScriptAddressGenerator>();

SpendDataType generateFromScriptAddress(const blocksci::AnyScript &scriptData, blocksci::AddressType::Enum type, const blocksci::ScriptAccess &scripts) {
    auto index = static_cast<size_t>(type);
    if (index >= blocksci::AddressType::size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return scriptAddressGeneratorTable[index](scriptData, scripts);
}

AnySpendData::AnySpendData(const AnyScriptOutput &output) : wrapped(boost::apply_visitor(SpendDataGenerator(), output.wrapped)) {}

AnySpendData::AnySpendData(const blocksci::AnyScript &scriptData, blocksci::AddressType::Enum addressType, const blocksci::ScriptAccess &scripts) : wrapped(generateFromScriptAddress(scriptData, addressType, scripts)) {}

SpendData<blocksci::AddressType::Enum::MULTISIG>::SpendData(const ScriptOutput<blocksci::AddressType::Enum::MULTISIG> &output) {
    uint32_t i = 0;
    for (auto &address : output.data.addresses) {
        addresses[i] = address.data.pubkey;
        i++;
    }
    addressCount = i;
}

SpendData<blocksci::AddressType::Enum::MULTISIG>::SpendData(const blocksci::ScriptAddress<blocksci::ScriptType::Enum::MULTISIG> &output, const blocksci::ScriptAccess &scripts) {
    uint32_t i = 0;
    for (auto address : output.addresses) {
        addresses[i] = *blocksci::script::Pubkey(scripts, address.scriptNum).getPubkey();
        i++;
    }
    addressCount = i;
}
