//
//  utxo_address_state.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#include "utxo_address_state.hpp"
#include "script_output.hpp"

void UTXOAddressState::addOutput(const AnySpendData &spendData, const blocksci::OutputPointer &pointer) {
    mpark::visit([&](const auto &spendData) { this->addOutput(spendData, pointer); }, spendData.wrapped);
}

template<blocksci::AddressType::Enum type>
struct SpendOutputFunctor {
    static SpendDataType f(const blocksci::OutputPointer &pointer, UTXOAddressState &state) {
        return state.spendOutput<type>(pointer);
    }
};

static auto spendOutputTable = blocksci::make_dynamic_table<blocksci::AddressType, SpendOutputFunctor>();

AnySpendData UTXOAddressState::spendOutput(const blocksci::OutputPointer &pointer, blocksci::AddressType::Enum type) {
    auto index = static_cast<size_t>(type);
    if (index >= blocksci::AddressType::size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return AnySpendData{spendOutputTable.at(index)(pointer, *this)};
}

void UTXOAddressState::unserialize(const boost::filesystem::path &path) {
    blocksci::for_each(addressTypeStates, [&](auto &addressTypeState) {
        std::stringstream ss;
        ss << addressName(addressTypeState.type);
        ss << ".dat";
        auto fullPath = path / ss.str();
        addressTypeState.unserialize(fullPath.native());
    });
}

void UTXOAddressState::serialize(const boost::filesystem::path &path) {
    blocksci::for_each(addressTypeStates, [&](auto &addressTypeState) {
        std::stringstream ss;
        ss << addressName(addressTypeState.type);
        ss << ".dat";
        auto fullPath = path / ss.str();
        addressTypeState.serialize(fullPath.native());
    });
}
