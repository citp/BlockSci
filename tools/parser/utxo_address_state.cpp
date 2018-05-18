//
//  utxo_address_state.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#include "utxo_address_state.hpp"
#include "script_output.hpp"

#include <wjfilesystem/path.h>

void UTXOAddressState::addOutput(const AnySpendData &spendData, const blocksci::InoutPointer &pointer) {
    mpark::visit([&](const auto &spendData) { this->addOutput(spendData, pointer); }, spendData.wrapped);
}

template<blocksci::AddressType::Enum type>
struct SpendOutputFunctor {
    static SpendDataType f(const blocksci::InoutPointer &pointer, UTXOAddressState &state) {
        return state.spendOutput<type>(pointer);
    }
};

static auto spendOutputTable = blocksci::make_dynamic_table<blocksci::AddressType, SpendOutputFunctor>();

AnySpendData UTXOAddressState::spendOutput(const blocksci::InoutPointer &pointer, blocksci::AddressType::Enum type) {
    auto index = static_cast<size_t>(type);
    if (index >= blocksci::AddressType::size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return AnySpendData{spendOutputTable.at(index)(pointer, *this)};
}

void UTXOAddressState::unserialize(const std::string &path) {
    blocksci::for_each(addressTypeStates, [&](auto &addressTypeState) {
        std::stringstream ss;
        ss << addressName(addressTypeState.type);
        ss << ".dat";
        auto fullPath = filesystem::path{path} / ss.str();
        addressTypeState.unserialize(fullPath.str());
    });
}

void UTXOAddressState::serialize(const std::string &path) {
    blocksci::for_each(addressTypeStates, [&](auto &addressTypeState) {
        std::stringstream ss;
        ss << addressName(addressTypeState.type);
        ss << ".dat";
        auto fullPath = filesystem::path{path} / ss.str();
        addressTypeState.serialize(fullPath.str());
    });
}
