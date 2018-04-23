//
//  data_configuration.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef data_configuration_h
#define data_configuration_h

#include <blocksci/blocksci_export.h>
#include <blocksci/typedefs.hpp>

#include <string>
#include <vector>

namespace blocksci {
    
    static constexpr int dataVersion = 4;
    
    struct BLOCKSCI_EXPORT DataConfiguration {
        DataConfiguration() {}
        // May create data directory (Used by parser)
        explicit DataConfiguration(const std::string &dataDirectory);
        
        // Must point to existing data directory
        DataConfiguration(const std::string &dataDirectory, bool errorOnReorg, BlockHeight blocksIgnored);
        
        bool errorOnReorg;
        BlockHeight blocksIgnored;
        
        std::vector<unsigned char> pubkeyPrefix;
        std::vector<unsigned char> scriptPrefix;
        std::string segwitPrefix;
        
        std::string dataDirectory;
        
        bool isNull() const {
            return dataDirectory.empty();
        }
        
        std::string scriptsDirectory() const;
        
        std::string chainDirectory() const;
        
        std::string mempoolDirectory() const;
        
        std::string addressDBFilePath() const;
        std::string hashIndexFilePath() const;
        
        bool operator==(const DataConfiguration &other) const {
            return dataDirectory == other.dataDirectory;
        }
        
        bool operator!=(const DataConfiguration &other) const {
            return dataDirectory != other.dataDirectory;
        }
    };

}

#endif /* data_configuration_h */
