//
//  raw_address_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/17/17.
//
//

#ifndef raw_address_pointer_hpp
#define raw_address_pointer_hpp

#include "address_types.hpp"
#include <blocksci/bitcoin_uint256.hpp>

#include <boost/optional/optional_fwd.hpp>
#include <boost/functional/hash.hpp>

namespace blocksci {
    struct DataConfiguration;
    struct PubkeyData;
    struct ScriptHashData;
    
    struct RawAddress {
        static constexpr size_t maxAddressSize = 40;
        static constexpr size_t maxBinSize = 25;
        
        uint160 hash;
        AddressType::Enum type;
        
        bool operator==(const RawAddress& other) const {
            return type == other.type && hash == other.hash;
        }
        
        RawAddress() {}
        RawAddress(uint160 hash, AddressType::Enum type);
        RawAddress(const PubkeyData &data);
        RawAddress(const ScriptHashData &data);
        
        static boost::optional<RawAddress> create(const DataConfiguration &config, const std::string &addressString);
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        static boost::optional<RawAddress> create(const std::string &addressString);
        #endif
    };
}

namespace std {
    template <>
    struct hash<blocksci::RawAddress> {
    public:
        size_t operator()(const blocksci::RawAddress &b) const {
            std::size_t seed = 9765487;
            boost::hash_combine(seed, b.hash);
            boost::hash_combine(seed, b.type);
            return seed;
        }
    };
}


#endif /* raw_address_pointer_hpp */
