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
#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/typedefs.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <sstream>

namespace blocksci {
    
    static constexpr int dataVersion = 4;
    
    struct BLOCKSCI_EXPORT DataConfiguration {
        DataConfiguration() {}
        // May create data directory (Used by parser)
        explicit DataConfiguration(boost::filesystem::path dataDirectory);
        
        // Must point to existing data directory
        DataConfiguration(boost::filesystem::path dataDirectory, bool errorOnReorg, BlockHeight blocksIgnored);
        
        bool errorOnReorg;
        BlockHeight blocksIgnored;
        
        std::vector<unsigned char> pubkeyPrefix;
        std::vector<unsigned char> scriptPrefix;
        std::string segwitPrefix;
        
        boost::filesystem::path dataDirectory;
        
        bool isNull() const {
            return dataDirectory.empty();
        }
        
        boost::filesystem::path scriptsDirectory() const {
            return dataDirectory/"scripts";
        }
        
        boost::filesystem::path chainDirectory() const {
            return dataDirectory/"chain";
        }
        
        boost::filesystem::path mempoolDirectory() const {
            return dataDirectory/"mempool";
        }
        
        boost::filesystem::path txFilePath() const {
            return chainDirectory()/"tx";
        }
        
        boost::filesystem::path txHashesFilePath() const {
            return chainDirectory()/"tx_hashes";
        }
        
        boost::filesystem::path blockFilePath() const {
            return chainDirectory()/"block";
        }
        
        boost::filesystem::path blockCoinbaseFilePath() const {
            return chainDirectory()/"coinbases";
        }
        
        boost::filesystem::path sequenceFilePath() const {
            return chainDirectory()/"sequence";
        }
        
        boost::filesystem::path addressDBFilePath() const {
            return dataDirectory/"addressesDb";
        }
        
        boost::filesystem::path hashIndexFilePath() const {
            return dataDirectory/"hashIndex";
        }
        
        boost::filesystem::path scriptTypeCountFile() const {
            return chainDirectory()/"scriptTypeCount.txt";
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
