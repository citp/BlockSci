//
//  output_spend_data.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#ifndef output_spend_data_hpp
#define output_spend_data_hpp

#include "parser_fwd.hpp"


#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/bitcoin_pubkey.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <mpark/variant.hpp>

template<blocksci::AddressType::Enum type>
struct SpendData {
    static constexpr auto address_v = type;
    
    SpendData() = default;
    SpendData(const ScriptOutput<type> &) {}
    SpendData(const blocksci::ScriptAddress<type> &) {}
};

template<>
struct SpendData<blocksci::AddressType::Enum::MULTISIG> {
    static constexpr auto address_v = blocksci::AddressType::Enum::MULTISIG;
    
    uint32_t addressCount;
    std::array<blocksci::CPubKey, 16> addresses;
    
    SpendData() = default;
    SpendData(const ScriptOutput<blocksci::AddressType::Enum::MULTISIG> &output);
    SpendData(const blocksci::ScriptAddress<blocksci::AddressType::Enum::MULTISIG> &output);
};

using SpendDataType = blocksci::to_variadic_t<blocksci::to_address_tuple_t<SpendData>, mpark::variant>;

class AnySpendData {
public:
    SpendDataType wrapped;
    
    AnySpendData(const SpendDataType &var) : wrapped(var) {}
    
    AnySpendData(const AnyScriptOutput &scriptOutput);
    AnySpendData(const blocksci::AnyScript &scriptData, blocksci::AddressType::Enum addressType);
};


#endif /* output_spend_data_hpp */
