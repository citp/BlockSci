//
//  utxo_address_state.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/29/17.
//

#ifndef utxo_address_state_hpp
#define utxo_address_state_hpp

#include "parser_fwd.hpp"
#include "output_spend_data.hpp"
#include "serializable_map.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/address/address_info.hpp>
#include <boost/filesystem/path.hpp>

template<auto addressType>
class UTXOAddressTypeState {
    SerializableMap<blocksci::OutputPointer, SpendData<addressType>> map;
public:
    
    static constexpr auto type = addressType;
    
    UTXOAddressTypeState() : map({0, 0}, {0, 1}) {}
    
    void unserialize(const std::string &path) {
        map.unserialize(path);
    }
    
    void serialize(const std::string &path) {
        map.serialize(path);
    }
    
    SpendData<type> spendOutput(const blocksci::OutputPointer &pointer) {
        if constexpr(std::is_empty_v<SpendData<addressType>>) {
            return SpendData<type>();
        } else {
            return map.erase(pointer);
        }
    }
    
    void addOutput(const SpendData<type> &spendData, const blocksci::OutputPointer &outputPointer) {
        if constexpr(!std::is_empty_v<SpendData<addressType>>) {
            map.add(outputPointer, spendData);
        }
    }
};

class UTXOAddressState {
private:
    
    using UTXOAddressTypeStateTuple = blocksci::to_address_tuple_t<UTXOAddressTypeState>;
    
    UTXOAddressTypeStateTuple addressTypeStates;
    
public:
    
    void unserialize(const boost::filesystem::path &path);
    void serialize(const boost::filesystem::path &path);
    
    AnySpendData spendOutput(const blocksci::OutputPointer &outputPointer, blocksci::AddressType::Enum type);
    void addOutput(const AnySpendData &spendData, const blocksci::OutputPointer &outputPointer);
    
    template<auto type>
    void addOutput(const SpendData<type> &spendData, const blocksci::OutputPointer &outputPointer) {
        std::get<UTXOAddressTypeState<type>>(addressTypeStates).addOutput(spendData, outputPointer);
    }
    
    template<auto type>
    SpendData<type> spendOutput(const blocksci::OutputPointer &outputPointer) {
        return std::get<UTXOAddressTypeState<type>>(addressTypeStates).spendOutput(outputPointer);
    }
};

#endif /* utxo_address_state_hpp */
