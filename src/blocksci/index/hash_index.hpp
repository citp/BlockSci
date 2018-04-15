//
//  hash_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/27/17.
//
//

#ifndef blocksci_index_hash_index_hpp
#define blocksci_index_hash_index_hpp

#include "column_iterator.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_info.hpp>
#include <blocksci/util/memory_view.hpp>

#include <range/v3/view_facade.hpp>
#include <range/v3/view/transform.hpp>

#include <array>
#include <vector>
#include <cstdint>

namespace blocksci {
    struct State;
    class HashIndexPriv;
    
    class BLOCKSCI_EXPORT HashIndex {
        
        std::unique_ptr<HashIndexPriv> impl;
        
        uint32_t lookupAddressImpl(blocksci::AddressType::Enum type, const char *data, size_t size);
        
    public:
        
        HashIndex(const std::string &path, bool readonly);
        ~HashIndex();
        
        template<AddressType::Enum type>
        uint32_t lookupAddress(const typename AddressInfo<type>::IDType &hash) {
            return lookupAddressImpl(type, reinterpret_cast<const char *>(&hash), sizeof(hash));
        }
        
        uint32_t getPubkeyHashIndex(const uint160 &pubkeåyhash);
        uint32_t getScriptHashIndex(const uint160 &scripthash);
        uint32_t getScriptHashIndex(const uint256 &scripthash);
        uint32_t getTxIndex(const uint256 &txHash);
        
        uint32_t countColumn(AddressType::Enum type);
        uint32_t countTxes();
        
        void compactDB();
        
        void addAddressesImpl(AddressType::Enum type, std::vector<std::pair<MemoryView, MemoryView>> dataViews);
        
        template<AddressType::Enum type>
        void addAddresses(std::vector<std::pair<typename blocksci::AddressInfo<type>::IDType, uint32_t>> rows) {
            std::vector<std::pair<MemoryView, MemoryView>> dataViews;
            for (const auto &pair : rows) {
                dataViews.emplace_back(
                MemoryView{reinterpret_cast<const char *>(&pair.first), sizeof(pair.first)},
                MemoryView{reinterpret_cast<const char *>(&pair.second), sizeof(pair.second)});
            }
            addAddressesImpl(type, dataViews);
        }
        
        void addTxes(std::vector<std::pair<uint256, uint32_t>> rows);
        
        void rollback(const blocksci::State &state);
        
        ColumnIterator getRawAddressRange(AddressType::Enum type);
        
        template<AddressType::Enum type>
        auto getAddressRange() {
            using IDType = typename blocksci::AddressInfo<type>::IDType;
            return getRawAddressRange(type) | ranges::view::transform([](std::pair<MemoryView, MemoryView> pair) -> std::pair<uint32_t, IDType> {
                auto &key = pair.first;
                auto &value = pair.second;
                uint32_t addressNum;
                IDType hash;
                memcpy(&addressNum, value.data, sizeof(addressNum));
                memcpy(&hash, key.data, sizeof(hash));
                return {addressNum, hash};
            });
        }
    };
}

#endif /* blocksci_index_hash_index_hpp */
