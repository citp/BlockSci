//
//  data_configuration.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef data_configuration_h
#define data_configuration_h

#include "chain_configuration.hpp"

#include <blocksci/core/typedefs.hpp>

#include <wjfilesystem/path.h>

#include <string>
#include <vector>

namespace blocksci {
    
    static constexpr int dataVersion = 5;
    
    nlohmann::json loadConfig(const std::string &configFilePath);
    void checkVersion(const nlohmann::json &jsonConf);
    
    struct DataConfiguration {
        DataConfiguration() {}
        DataConfiguration(ChainConfiguration &config, bool errorOnReorg, BlockHeight blocksIgnored);
        
        bool errorOnReorg;
        BlockHeight blocksIgnored;
        
        ChainConfiguration chainConfig;
        
        bool isNull() const {
            return chainConfig.dataDirectory.empty();
        }
        
        filesystem::path scriptsDirectory() const {
            return chainConfig.dataDirectory/"scripts";
        }
        
        filesystem::path chainDirectory() const {
            return chainConfig.dataDirectory/"chain";
        }
        
        filesystem::path mempoolDirectory() const {
            return chainConfig.dataDirectory/"mempool";
        }
        
        filesystem::path addressDBFilePath() const {
            return chainConfig.dataDirectory/"addressesDb";
        }
        
        filesystem::path hashIndexFilePath() const {
            return chainConfig.dataDirectory/"hashIndex";
        }
        
        bool operator==(const DataConfiguration &other) const {
            return chainConfig.dataDirectory == other.chainConfig.dataDirectory;
        }
        
        bool operator!=(const DataConfiguration &other) const {
            return chainConfig.dataDirectory != other.chainConfig.dataDirectory;
        }
    };
    
    DataConfiguration loadBlockchainConfig(const std::string &configPath, bool errorOnReorg, BlockHeight blocksIgnored);
}

#endif /* data_configuration_h */
