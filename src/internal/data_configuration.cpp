//
//  data_configuration.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "data_configuration.hpp"

#include <simpleini/SimpleIni.h>

namespace blocksci {
    
    void createDirectory(const filesystem::path &dir) {
        if(!dir.exists()){
            filesystem::create_directory(dir);
        }
    }
    
    DataConfiguration::DataConfiguration(const std::string &dataDirectory_) : errorOnReorg(false), blocksIgnored(0), dataDirectory(filesystem::path{dataDirectory_}.str()) {
        createDirectory(dataDirectory);
        createDirectory(scriptsDirectory());
        createDirectory(chainDirectory());
        createDirectory(mempoolDirectory());
        
        auto configFile = filesystem::path{dataDirectory}/"config.ini";
        if (configFile.exists()) {
            CSimpleIniA ini;
            ini.SetUnicode();
            ini.LoadFile(configFile.str().c_str());
            auto versionNum = ini.GetLongValue("", "version");
            if (versionNum != dataVersion) {
                throw std::runtime_error("Error, parser data is not in the correct format. To fix you must delete the data file and rerun the parser");
            }
        }
    }
    
    DataConfiguration::DataConfiguration(const std::string &dataDirectory_, bool errorOnReorg_, BlockHeight blocksIgnored_) : errorOnReorg(errorOnReorg_), blocksIgnored(blocksIgnored_), dataDirectory(dataDirectory_) {
        if(!(dataDirectory.exists())){
            throw std::runtime_error("Error, blocksci data directory does not exist");
        }
        
        if(!(scriptsDirectory().exists())){
            throw std::runtime_error("Error, blocksci scripts directory does not exist");
        }
        
        if(!(chainDirectory().exists())){
            throw std::runtime_error("Error, blocksci chain directory does not exist");
        }
        
        auto configFile = dataDirectory/"config.ini";
        if (configFile.exists()) {
            CSimpleIniA ini;
            ini.SetUnicode();
            ini.LoadFile(configFile.str().c_str());
            auto versionNum = ini.GetLongValue("", "version");
            if (versionNum != dataVersion) {
                throw std::runtime_error("Error, parser data is not in the correct format. To fix you must delete the data file and rerun the parser");
            }
        } else {
            std::stringstream ss;
            ss << "Error, data directory does not contain config.ini. Are you sure " << dataDirectory << " was the output directory of blocksci_parser?";
            throw std::runtime_error(ss.str());
        }
        
        auto dataDirectoryStr = dataDirectory.str();
        
        if (dataDirectoryStr.find("dash_testnet") != std::string::npos) {
            chainConfig = ChainConfiguration::dashTestNet();
        } else if(dataDirectoryStr.find("dash") != std::string::npos) {
            chainConfig = ChainConfiguration::dash();
        } else if(dataDirectoryStr.find("litecoin") != std::string::npos) {
            chainConfig = ChainConfiguration::litecoin();
        } else if(dataDirectoryStr.find("zcash") != std::string::npos) {
            chainConfig = ChainConfiguration::zcash();
        } else if(dataDirectoryStr.find("namecoin") != std::string::npos) {
            chainConfig = ChainConfiguration::namecoin();
        } else if(dataDirectoryStr.find("bitcoin_regtest") != std::string::npos) {
            chainConfig = ChainConfiguration::bitcoinRegtest();
        } else {
            chainConfig = ChainConfiguration::bitcoin();
        }
    }
}
