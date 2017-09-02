//
//  raw_address_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/17/17.
//
//

#ifndef raw_address_pointer_hpp
#define raw_address_pointer_hpp

#include "script_types.hpp"
#include <blocksci/uint256.hpp>

#include <boost/optional.hpp>

#include <stdio.h>

namespace blocksci {
    struct DataConfiguration;
    struct PubkeyHashData;
    struct ScriptHashData;
    
    struct RawAddress {
        static constexpr size_t maxAddressSize = 40;
        static constexpr size_t maxBinSize = 25;
        
        uint160 hash;
        ScriptType::Enum type;
        
        bool operator==(const RawAddress& other) const {
            return type == other.type && hash == other.hash;
        }
        
        RawAddress() {}
        RawAddress(uint160 hash, ScriptType::Enum type);
        RawAddress(const PubkeyHashData &data);
        RawAddress(const ScriptHashData &data);
        
        static boost::optional<RawAddress> create(const DataConfiguration &config, const std::string &addressString);
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        static boost::optional<RawAddress> create(const std::string &addressString);
        #endif
    };
}

namespace std {
    template <>
    class hash<blocksci::RawAddress> {
    public:
        size_t operator()(const blocksci::RawAddress &b) const {
            std::size_t seed = 123945432;
            hash<blocksci::uint160> hasher1;
            hash<blocksci::ScriptType::Enum> hasher2;
            seed ^= hasher1(b.hash) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            seed ^= hasher2(b.type) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            return seed;
        }
    };
}


#endif /* raw_address_pointer_hpp */
