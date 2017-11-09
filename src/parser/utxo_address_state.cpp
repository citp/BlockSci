//
//  utxo_address_state.cpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#include "utxo_address_state.hpp"
#include "script_output.hpp"

struct AddOutputVisitor : public boost::static_visitor<void> {
    UTXOAddressState &state;
    const blocksci::OutputPointer &pointer;
    AddOutputVisitor(UTXOAddressState &state_, const blocksci::OutputPointer &pointer_) : state(state_), pointer(pointer_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(const SpendData<type> &spendData) const {
        state.addOutput(spendData, pointer);
    }
};

void UTXOAddressState::addOutput(const AnySpendData &spendData, const blocksci::OutputPointer &pointer) {
    AddOutputVisitor visitor(*this, pointer);
    boost::apply_visitor(visitor, spendData.wrapped);
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
        return spendOutputTable[index](pointer, *this);
}

void UTXOAddressState::unserialize(const boost::filesystem::path &path) {
    blocksci::for_each(addressTypeStates, [&](auto &addressTypeState) {
        std::stringstream ss;
        ss << addressName(addressTypeState.type);
        ss << ".dat";
        addressTypeState.unserialize(path / ss.str());
    });
}

void UTXOAddressState::serialize(const boost::filesystem::path &path) {
    blocksci::for_each(addressTypeStates, [&](auto &addressTypeState) {
        std::stringstream ss;
        ss << addressName(addressTypeState.type);
        ss << ".dat";
        addressTypeState.serialize(path / ss.str());
    });
}
