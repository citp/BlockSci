//
//  data_configuration.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include <blocksci/util/data_configuration.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace blocksci {
    
    void createDirectory(const boost::filesystem::path &dir) {
        if(!(boost::filesystem::exists(dir))){
            boost::filesystem::create_directory(dir);
        }
    }
    
    DataConfiguration::DataConfiguration(const std::string &dataDirectory_) : errorOnReorg(false), blocksIgnored(0), dataDirectory(boost::filesystem::path{dataDirectory_}.native()) {
        createDirectory(dataDirectory);
        createDirectory(scriptsDirectory());
        createDirectory(chainDirectory());
        createDirectory(mempoolDirectory());
        
        boost::property_tree::ptree root;
        auto configFile = boost::filesystem::path{dataDirectory}/"config.ini";
        if (boost::filesystem::exists(configFile)) {
            boost::filesystem::ifstream configStream{configFile};
            boost::property_tree::read_ini(configStream, root);
            auto versionNum = root.get("version", 0);
            if (versionNum != dataVersion) {
                throw std::runtime_error("Error, parser data is not in the correct format. To fix you must delete the data file and rerun the parser");
            }
        }
    }
    
    DataConfiguration::DataConfiguration(const std::string &dataDirectory_, bool errorOnReorg_, BlockHeight blocksIgnored_) : errorOnReorg(errorOnReorg_), blocksIgnored(blocksIgnored_), dataDirectory(boost::filesystem::path{dataDirectory_}.native()) {
        if(!(boost::filesystem::exists(dataDirectory))){
            throw std::runtime_error("Error, blocksci data directory does not exist");
        }
        
        if(!(boost::filesystem::exists(scriptsDirectory()))){
            throw std::runtime_error("Error, blocksci scripts directory does not exist");
        }
        
        if(!(boost::filesystem::exists(chainDirectory()))){
            throw std::runtime_error("Error, blocksci chain directory does not exist");
        }
        
        boost::property_tree::ptree root;
        auto configFile = boost::filesystem::path{dataDirectory}/"config.ini";
        if (boost::filesystem::exists(configFile)) {
            boost::filesystem::ifstream configStream{configFile};
            boost::property_tree::read_ini(configStream, root);
            auto versionNum = root.get("version", 0);
            if (versionNum != dataVersion) {
                throw std::runtime_error("Error, parser data is not in the correct format. To fix you must delete the data file and rerun the parser");
            }
        } else {
            std::stringstream ss;
            ss << "Error, data directory does not contain config.ini. Are you sure " << dataDirectory << " was the output directory of blocksci_parser?";
            throw std::runtime_error(ss.str());
        }
        
        if (dataDirectory.find("dash_testnet") != std::string::npos) {
            pubkeyPrefix = {140};
            scriptPrefix = {19};
            segwitPrefix = "NONE";
        } else if(dataDirectory.find("dash") != std::string::npos) {
            pubkeyPrefix = {76};
            scriptPrefix = {16};
            segwitPrefix = "NONE";
        } else if(dataDirectory.find("litecoin") != std::string::npos) {
            pubkeyPrefix = {48};
            scriptPrefix = {50};
            segwitPrefix = "ltc";
        } else if(dataDirectory.find("zcash") != std::string::npos) {
            pubkeyPrefix = {28,184};
            scriptPrefix = {28,189};
            segwitPrefix = "NONE";
        } else if(dataDirectory.find("namecoin") != std::string::npos) {
            pubkeyPrefix = {52};
            scriptPrefix = {13};
            segwitPrefix = "nc";
        } else {
            pubkeyPrefix = std::vector<unsigned char>(1,0);
            scriptPrefix = std::vector<unsigned char>(1,5);
            segwitPrefix = "bc";
        }
    }
    
    std::string DataConfiguration::scriptsDirectory() const {
        return (boost::filesystem::path{dataDirectory}/"scripts").native();
    }
    
    std::string DataConfiguration::chainDirectory() const {
        return (boost::filesystem::path{dataDirectory}/"chain").native();
    }
    
    std::string DataConfiguration::mempoolDirectory() const {
        return (boost::filesystem::path{dataDirectory}/"mempool").native();
    }
    
    std::string DataConfiguration::addressDBFilePath() const {
        return (boost::filesystem::path{dataDirectory}/"addressesDb").native();
    }
    
    std::string DataConfiguration::hashIndexFilePath() const {
        return (boost::filesystem::path{dataDirectory}/"hashIndex").native();
    }
    
}
