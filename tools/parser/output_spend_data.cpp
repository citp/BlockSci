//
//  output_spend_data.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#include "output_spend_data.hpp"
#include "script_output.hpp"

#include <blocksci/scripts/script_variant.hpp>

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

template<blocksci::AddressType::Enum type>
struct RawAddressSpendDataFunctor {
    static SpendDataType f(const blocksci::RawAddress &address, const blocksci::ScriptAccess &scripts) {
        return SpendData<type>(address, scripts);
    }
};

SpendDataType rawAddressSpendData(const blocksci::RawAddress &address, const blocksci::ScriptAccess &scripts) {
    static auto &rawAddressSpendDataTable = *[]() {
        auto nameTable = blocksci::make_dynamic_table<blocksci::AddressType, RawAddressSpendDataFunctor>();
        return new decltype(nameTable){nameTable};
    }();
    return rawAddressSpendDataTable.at(static_cast<size_t>(address.type))(address,scripts);
}

AnySpendData::AnySpendData(const AnyScriptOutput &output) : wrapped(mpark::visit(SpendDataGenerator(), output.wrapped)) {}

AnySpendData::AnySpendData(const blocksci::AnyScript &scriptData) : wrapped(generateFromScriptAddress(scriptData)) {}

AnySpendData::AnySpendData(const blocksci::RawAddress &address, const blocksci::ScriptAccess &scripts) : wrapped(rawAddressSpendData(address, scripts)) {}

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

SpendData<blocksci::AddressType::Enum::MULTISIG>::SpendData(const blocksci::RawAddress &address, const blocksci::ScriptAccess &scripts) {
    uint32_t i = 0;
    auto scriptData = scripts.getScriptData<blocksci::DedupAddressType::MULTISIG>(address.scriptNum);
    for (auto addressNum : scriptData->addresses) {
        addresses.at(i) = scripts.getScriptData<blocksci::DedupAddressType::PUBKEY>(addressNum)->pubkey;
        i++;
    }
}
