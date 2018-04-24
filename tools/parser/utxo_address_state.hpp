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

#include <blocksci/chain/inout_pointer.hpp>

template<blocksci::AddressType::Enum addressType>
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
    
    template<typename T = SpendData<addressType>, std::enable_if_t<std::is_empty<T>::value, int> = 0>
    SpendData<type> spendOutput(const blocksci::OutputPointer &) {
        return SpendData<type>();
    }
    
    template<typename T = SpendData<addressType>, std::enable_if_t<!std::is_empty<T>::value, int> = 0>
    SpendData<type> spendOutput(const blocksci::OutputPointer &pointer) {
        return map.erase(pointer);
    }
    
    template<typename T = SpendData<addressType>, std::enable_if_t<std::is_empty<T>::value, int> = 0>
    void addOutput(const SpendData<type> &, const blocksci::OutputPointer &) {
        
    }
    
    template<typename T = SpendData<addressType>, std::enable_if_t<!std::is_empty<T>::value, int> = 0>
    void addOutput(const SpendData<type> &spendData, const blocksci::OutputPointer &outputPointer) {
        map.add(outputPointer, spendData);
    }
};

class UTXOAddressState {
private:
    
    using UTXOAddressTypeStateTuple = blocksci::to_address_tuple_t<UTXOAddressTypeState>;
    
    UTXOAddressTypeStateTuple addressTypeStates;
    
public:
    
    void unserialize(const std::string &path);
    void serialize(const std::string &path);
    
    AnySpendData spendOutput(const blocksci::OutputPointer &outputPointer, blocksci::AddressType::Enum type);
    void addOutput(const AnySpendData &spendData, const blocksci::OutputPointer &outputPointer);
    
    template<blocksci::AddressType::Enum type>
    void addOutput(const SpendData<type> &spendData, const blocksci::OutputPointer &outputPointer) {
        std::get<UTXOAddressTypeState<type>>(addressTypeStates).addOutput(spendData, outputPointer);
    }
    
    template<blocksci::AddressType::Enum type>
    SpendData<type> spendOutput(const blocksci::OutputPointer &outputPointer) {
        return std::get<UTXOAddressTypeState<type>>(addressTypeStates).spendOutput(outputPointer);
    }
};

#endif /* utxo_address_state_hpp */
