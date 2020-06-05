//
//  blockchain.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blockchain_hpp
#define blockchain_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/block_range.hpp>

#include <map>
#include <type_traits>
#include <future>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    class BLOCKSCI_EXPORT Blockchain : public BlockRange {
        /** Pointer to the DataAccess instance that manages all data access objects (ChainAccess, ScriptAccess etc.) for this chain */
        std::unique_ptr<DataAccess> access;
    public:
        
        Blockchain() = default;
        Blockchain(std::unique_ptr<DataAccess> access_);
        explicit Blockchain(const DataConfiguration &config);
        explicit Blockchain(const std::string &configPath);
        Blockchain(const std::string &configPath, BlockHeight maxBlock);
        ~Blockchain();
        
        std::string dataLocation() const;
        std::string configLocation() const;

        void reload();
        bool isParserRunning();
        
        uint32_t addressCount(AddressType::Enum type) const;
    };
    
    uint32_t BLOCKSCI_EXPORT txCount(Blockchain &chain);
        
} // namespace blocksci


#endif /* blockchain_hpp */
