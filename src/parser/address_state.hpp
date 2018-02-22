//
//  address_state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#ifndef address_state_hpp
#define address_state_hpp

#include "bloom_filter.hpp"
#include "parser_fwd.hpp"
#include "serializable_map.hpp"

#include <blocksci/index/hash_index.hpp>
#include <blocksci/util/state.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/address/address_info.hpp>

#include <future>
#include <unordered_map>

class CBloomFilter;

enum class AddressLocation {
    MultiUseMap,
    LevelDb,
    NotFound
};

template<blocksci::AddressType::Enum type>
struct RawAddressInfo {
    blocksci::uint160 hash;
    AddressLocation location;
    uint32_t addressNum;
};

template<blocksci::AddressType::Enum type>
struct NonDudupAddressInfo {
    uint32_t addressNum;
};

template<blocksci::ScriptType::Enum scriptType>
constexpr int startingCount = 0;
template<>
constexpr int startingCount<blocksci::ScriptType::Enum::PUBKEY> = 600'000'000;
template<>
constexpr int startingCount<blocksci::ScriptType::Enum::SCRIPTHASH> = 100'000'000;
template<>
constexpr int startingCount<blocksci::ScriptType::Enum::MULTISIG> = 100'000'000;

class AddressState {
    static constexpr auto AddressFalsePositiveRate = .05;
    
    template<blocksci::ScriptType::Enum scriptType>
    class AddressMap : public SerializableMap<blocksci::uint160, uint32_t>  {
    public:
        static constexpr auto type = scriptType;
        AddressMap() : SerializableMap(blocksci::uint160S("FFFFFFFFFFFFFFFFFFFF"), blocksci::uint160S("AAAAAAAAAAAAAAAAAA")) {}
    };
    
    template<blocksci::ScriptType::Enum scriptType>
    class AddressBloomFilter : public BloomFilter  {
    public:
        static constexpr auto type = scriptType;
        AddressBloomFilter(const boost::filesystem::path &path) : BloomFilter(boost::filesystem::path(path).concat(scriptName(type)), startingCount<scriptType>, AddressFalsePositiveRate)  {}
    };
    
    boost::filesystem::path path;
    
    blocksci::HashIndex db;
    
    using AddressMapTuple = blocksci::to_script_tuple_t<AddressMap>;
    using AddressBloomFilterTuple = blocksci::to_script_tuple_t<AddressBloomFilter>;
    
    AddressMapTuple multiAddressMaps;
    AddressBloomFilterTuple addressBloomFilters;
    
    mutable long bloomNegativeCount = 0;
    mutable long multiCount = 0;
    mutable long dbCount = 0;
    mutable long bloomFPCount = 0;
    
    
    std::vector<uint32_t> scriptIndexes;
    
    template<blocksci::ScriptType::Enum type>
    void reloadBloomFilter() {
        auto &addressBloomFilter = std::get<AddressBloomFilter<type>>(addressBloomFilters);
        addressBloomFilter.reset(addressBloomFilter.getMaxItems(), addressBloomFilter.getFPRate());
        rocksdb::Iterator* it = db.getIterator(type);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            uint32_t scriptNum;
            memcpy(&scriptNum, it->value().data(), sizeof(scriptNum));
            blocksci::uint160 addressHash;
            memcpy(&addressHash, it->key().data(), sizeof(addressHash));
            addressBloomFilter.add(addressHash);
        }
    }
    
    // Duplicated to prevent triggering gcc
    void reloadBloomFilters() {
        blocksci::for_each(blocksci::ScriptInfoList(), [&](auto tag) {
            auto &addressBloomFilter = std::get<AddressBloomFilter<tag>>(addressBloomFilters);
            addressBloomFilter.reset(addressBloomFilter.getMaxItems(), addressBloomFilter.getFPRate());
            rocksdb::Iterator* it = db.getIterator(tag);
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                uint32_t scriptNum;
                memcpy(&scriptNum, it->value().data(), sizeof(scriptNum));
                blocksci::uint160 addressHash;
                memcpy(&addressHash, it->key().data(), sizeof(addressHash));
                addressBloomFilter.add(addressHash);
            }
        });
    }
    
public:
    AddressState(const boost::filesystem::path &path, const boost::filesystem::path &hashIndexPath);
    AddressState(const AddressState &) = delete;
    AddressState &operator=(const AddressState &) = delete;
    AddressState(AddressState &&) = delete;
    AddressState &operator=(AddressState &&) = delete;
    ~AddressState();
    
    template<blocksci::AddressType::Enum type, std::enable_if_t<!blocksci::ScriptInfo<scriptType(type)>::deduped, int> = 0>
    NonDudupAddressInfo<type> findAddress(const ScriptOutputData<type> &) {
        uint32_t scriptNum = getNewAddressIndex(scriptType(type));
        return NonDudupAddressInfo<type>{scriptNum};
    }
    
    template<blocksci::AddressType::Enum type>
    std::pair<uint32_t, bool> resolveAddress(const NonDudupAddressInfo<type> &addressInfo) {
        return std::make_pair(addressInfo.addressNum, true);
    }
    
    template<blocksci::AddressType::Enum type, std::enable_if_t<blocksci::ScriptInfo<scriptType(type)>::deduped, int> = 0>
    RawAddressInfo<type> findAddress(const ScriptOutputData<type> &data) {
        auto hash = data.getHash();
        auto &addressBloomFilter = std::get<AddressBloomFilter<scriptType(type)>>(addressBloomFilters);
        if (!addressBloomFilter.possiblyContains(hash)) {
            // Address has definitely never been seen
            bloomNegativeCount++;
            return {hash, AddressLocation::NotFound, 0};
        }
        
        {
            auto &multiAddressMap = std::get<AddressMap<scriptType(type)>>(multiAddressMaps);
            auto it = multiAddressMap.find(hash);
            if (it != multiAddressMap.end()) {
                multiCount++;
                return {hash, AddressLocation::MultiUseMap, it->second};
            }
        }
        
        uint32_t destNum = db.lookupAddress<blocksci::AddressInfo<type>::dedupType>(hash);
        if (destNum != 0) {
            dbCount++;
            return {hash, AddressLocation::LevelDb, destNum};
        } else {
            bloomFPCount++;
            // We must have had a false positive
            return {hash, AddressLocation::NotFound, 0};
        }
    }
    
    // Bool is true if address is new
    template<blocksci::AddressType::Enum type>
    std::pair<uint32_t, bool> resolveAddress(const RawAddressInfo<type> &addressInfo) {
        bool existingAddress = false;
        switch (addressInfo.location) {
            case AddressLocation::LevelDb: {
                auto &multiAddressMap = std::get<AddressMap<scriptType(type)>>(multiAddressMaps);
                multiAddressMap.add(addressInfo.hash, addressInfo.addressNum);
                existingAddress = true;
                break;
            }
            case AddressLocation::MultiUseMap: {
                existingAddress = true;
                break;
            }
            case AddressLocation::NotFound: {
                existingAddress = false;
                break;
            }
        }
        
        uint32_t addressNum = addressInfo.addressNum;
        if (!existingAddress) {
            addressNum = getNewAddressIndex(scriptType(type));
            auto &addressBloomFilter = std::get<AddressBloomFilter<scriptType(type)>>(addressBloomFilters);
            addressBloomFilter.add(addressInfo.hash);
            db.addAddress<blocksci::AddressInfo<type>::dedupType>(addressInfo.hash, addressNum);
            if (addressBloomFilter.isFull()) {
                reloadBloomFilter<scriptType(type)>();
            }
        }
        return std::make_pair(addressNum, !existingAddress);
    }
    
    uint32_t getNewAddressIndex(blocksci::ScriptType::Enum type);
    
    void rollback(const blocksci::State &state);
};


#endif /* address_state_hpp */
