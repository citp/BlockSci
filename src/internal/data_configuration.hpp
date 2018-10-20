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
    
    struct DataConfiguration {
        DataConfiguration() {}
        // May create data directory (Used by parser)
        explicit DataConfiguration(const std::string &dataDirectory);
        
        // Must point to existing data directory
        DataConfiguration(const std::string &dataDirectory, bool errorOnReorg, BlockHeight blocksIgnored);
        
        bool errorOnReorg;
        BlockHeight blocksIgnored;
        
        ChainConfiguration chainConfig;
        
        filesystem::path dataDirectory;
        
        bool isNull() const {
            return dataDirectory.empty();
        }
        
        filesystem::path scriptsDirectory() const {
            return dataDirectory/"scripts";
        }
        
        filesystem::path chainDirectory() const {
            return dataDirectory/"chain";
        }
        
        filesystem::path mempoolDirectory() const {
            return dataDirectory/"mempool";
        }
        
        filesystem::path addressDBFilePath() const {
            return dataDirectory/"addressesDb";
        }
        
        filesystem::path hashIndexFilePath() const {
            return dataDirectory/"hashIndex";
        }
        
        bool operator==(const DataConfiguration &other) const {
            return dataDirectory == other.dataDirectory;
        }
        
        bool operator!=(const DataConfiguration &other) const {
            return dataDirectory != other.dataDirectory;
        }
    };

}

#endif /* data_configuration_h */
