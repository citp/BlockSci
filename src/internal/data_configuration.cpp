//
//  data_configuration.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "data_configuration.hpp"

#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::json;

namespace blocksci {
    
    DataConfiguration loadBlockchainConfig(const std::string &configPath, bool errorOnReorg, BlockHeight blocksIgnored) {
        auto jsonConf = loadConfig(configPath);
        checkVersion(jsonConf);
        
        ChainConfiguration chainConfig = jsonConf.at("chainConfig");
        return {configPath, chainConfig, errorOnReorg, blocksIgnored};
    }
    
    void createDirectory(const filesystem::path &dir) {
        if(!dir.exists()){
            filesystem::create_directory(dir);
        }
    }
    
    json loadConfig(const std::string &configFilePath) {
        filesystem::path configFile{configFilePath};
        
        if(!configFile.exists() || !configFile.is_file()) {
            std::stringstream ss;
            ss << "Error, path " << configFile.str() << " must point to to existing blocksci config file";
            throw std::runtime_error(ss.str());
        }
        
        std::ifstream rawConf(configFile.str());
        json jsonConf;
        rawConf >> jsonConf;
        return jsonConf;
    }

    // Checks whether the provided config file's version matches the config file version of the running program
    void checkVersion(const json &jsonConf) {
        uint64_t versionNum = jsonConf.at("version");
        if (versionNum != dataVersion) {
            throw std::runtime_error("Error, parser data is not in the correct format. To fix you must delete the data file and rerun the parser");
        }
    }
    
    DataConfiguration::DataConfiguration(const std::string &configPath_, ChainConfiguration &chainConfig_, bool errorOnReorg_, BlockHeight blocksIgnored_) : configPath(configPath_), errorOnReorg(errorOnReorg_), blocksIgnored(blocksIgnored_), chainConfig(chainConfig_) {
        createDirectory(chainConfig.dataDirectory);
        createDirectory(scriptsDirectory());
        createDirectory(chainDirectory());
        createDirectory(mempoolDirectory());
    }
}
