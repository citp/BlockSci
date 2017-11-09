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
#include <blocksci/scripts/scriptsfwd.hpp>

#include <boost/variant/variant.hpp>

template<auto type>
struct SpendData {
    SpendData() = default;
    SpendData(const ScriptOutput<type> &) {}
    SpendData(const blocksci::ScriptAddress<blocksci::scriptType(type)> &, const blocksci::ScriptAccess &) {}
};

template<>
struct SpendData<blocksci::AddressType::Enum::MULTISIG> {
    uint32_t addressCount;
    std::array<CPubKey, 16> addresses;
    
    SpendData() = default;
    SpendData(const ScriptOutput<blocksci::AddressType::Enum::MULTISIG> &output);
    SpendData(const blocksci::ScriptAddress<blocksci::ScriptType::Enum::MULTISIG> &output, const blocksci::ScriptAccess &scripts);
};

using SpendDataType = blocksci::to_address_variant_t<SpendData>;

class AnySpendData {
public:
    SpendDataType wrapped;
    
    AnySpendData(const SpendDataType &var) : wrapped(var) {}
    
    AnySpendData(const AnyScriptOutput &scriptOutput);
    AnySpendData(const blocksci::AnyScript &scriptData, blocksci::AddressType::Enum addressType, const blocksci::ScriptAccess &scripts);
};


#endif /* output_spend_data_hpp */
