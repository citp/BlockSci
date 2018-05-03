//
//  output_spend_data.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#ifndef output_spend_data_hpp
#define output_spend_data_hpp

#include "parser_fwd.hpp"

#include <blocksci/core/address_info.hpp>
#include <blocksci/scripts/bitcoin_pubkey.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <mpark/variant.hpp>

template<blocksci::AddressType::Enum type>
struct SpendData {
    static constexpr auto address_v = type;
    
    SpendData() = default;
    explicit SpendData(const ScriptOutput<type> &) {}
    explicit SpendData(const blocksci::ScriptAddress<type> &) {}
    SpendData(const blocksci::RawAddress &, const blocksci::ScriptAccess &) {}
};

template<>
struct SpendData<blocksci::AddressType::Enum::MULTISIG> {
    static constexpr auto address_v = blocksci::AddressType::Enum::MULTISIG;
    
    uint32_t addressCount;
    std::array<blocksci::CPubKey, 16> addresses;
    
    SpendData() = default;
    explicit SpendData(const ScriptOutput<blocksci::AddressType::Enum::MULTISIG> &output);
    explicit SpendData(const blocksci::ScriptAddress<blocksci::AddressType::Enum::MULTISIG> &output);
    SpendData(const blocksci::RawAddress &address, const blocksci::ScriptAccess &scripts);
};

using SpendDataType = blocksci::to_variadic_t<blocksci::to_address_tuple_t<SpendData>, mpark::variant>;

class AnySpendData {
public:
    SpendDataType wrapped;
    
    explicit AnySpendData(const SpendDataType &var) : wrapped(var) {}
    
    explicit AnySpendData(const AnyScriptOutput &scriptOutput);
    explicit AnySpendData(const blocksci::AnyScript &scriptData);
    AnySpendData(const blocksci::RawAddress &address, const blocksci::ScriptAccess &scripts);
};


#endif /* output_spend_data_hpp */
