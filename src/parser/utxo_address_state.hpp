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

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/address/address_info.hpp>

#include <boost/functional/hash.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include <google/dense_hash_map>

template<auto addressType>
class UTXOAddressTypeState {
    using utxo_map = google::dense_hash_map<blocksci::OutputPointer, SpendData<addressType>, std::hash<blocksci::OutputPointer>>;
    
    utxo_map utxoMap;
    
public:
    
    static constexpr auto type = addressType;
    
    UTXOAddressTypeState() {
        utxoMap.set_deleted_key({0, 0});
        utxoMap.set_empty_key({0, 1});
    }
    
    void unserialize(const boost::filesystem::path &path) {
        boost::filesystem::ifstream file{path, std::fstream::in | std::fstream::binary};
        if (file.is_open()) {
            typename utxo_map::NopointerSerializer serializer;
            auto success = utxoMap.unserialize(serializer, &file);
            assert(success);
        }
    }
    
    void serialize(const boost::filesystem::path &path) {
        boost::filesystem::ofstream file{path, std::fstream::out | std::fstream::binary};
        typename utxo_map::NopointerSerializer serializer;
        auto success = utxoMap.serialize(serializer, &file);
        assert(success);
    }
    
    struct UTXOAddressTypeMissingException : public std::runtime_error {
        UTXOAddressTypeMissingException(const blocksci::OutputPointer &pointer) : std::runtime_error(getMessage(pointer).c_str()) {}
        
    private:
        static std::string getMessage(const blocksci::OutputPointer &pointer) {
            std::stringstream ss;
            ss << "Tried to spend missing UTXO " << pointer;
            return ss.str();
        }
    };
    
    SpendData<type> spendOutput(const blocksci::OutputPointer &pointer) {
        if constexpr(std::is_empty_v<SpendData<addressType>>) {
            return SpendData<type>();
        } else {
            auto it = utxoMap.find(pointer);
            if (it == utxoMap.end()) {
                throw UTXOAddressTypeMissingException{pointer};
            }
            
            SpendData<type> spendData = it->second;
            utxoMap.erase(it);
            return spendData;
        }
    }
    
    void addOutput(const SpendData<type> &spendData, const blocksci::OutputPointer &outputPointer) {
        if constexpr(!std::is_empty_v<SpendData<addressType>>) {
            utxoMap.insert(std::make_pair(outputPointer, spendData));
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
