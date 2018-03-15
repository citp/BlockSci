//
//  data_configuration.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "util/data_configuration.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace blocksci {
    
    void createDirectory(boost::filesystem::path dir);
    
    
    void createDirectory(boost::filesystem::path dir) {
        if(!(boost::filesystem::exists(dir))){
            boost::filesystem::create_directory(dir);
        }
    }
    
    DataConfiguration::DataConfiguration(const boost::filesystem::path &dataDirectory_, bool errorOnReorg_, BlockHeight blocksIgnored_) : errorOnReorg(errorOnReorg_), blocksIgnored(blocksIgnored_), dataDirectory(dataDirectory_) {
        createDirectory(dataDirectory);
        createDirectory(scriptsDirectory());
        createDirectory(chainDirectory());
        
        boost::property_tree::ptree root;
        auto configFile = dataDirectory/"config.ini";
        if (boost::filesystem::exists(configFile)) {
            boost::filesystem::ifstream configStream{configFile};
            boost::property_tree::read_ini(configStream, root);
            auto versionNum = root.get("version", 0);
            if (versionNum != dataVersion) {
                throw std::runtime_error("Error, parser data is not in the correct format. To fix you must delete the data file and rerun the parser");
            }
        } else {
            std::stringstream ss;
            ss << "Error, data directory does not contain config.ini. Are you sure " << dataDirectory << "was the output directory of blocksci_parser?";
            throw std::runtime_error(ss.str());
        }
        
        auto dataDirectoryString = dataDirectory.native();
        if(dataDirectoryString.find("dash") != std::string::npos) {
            pubkeyPrefix = {76};
            scriptPrefix = {16};
            segwitPrefix = "NONE";
        } else if(dataDirectoryString.find("litecoin") != std::string::npos) {
            pubkeyPrefix = {48};
            scriptPrefix = {50};
            segwitPrefix = "ltc";
        } else if(dataDirectoryString.find("zcash") != std::string::npos) {
            pubkeyPrefix = {28,184};
            scriptPrefix = {28,189};
            segwitPrefix = "NONE";
        } else if(dataDirectoryString.find("namecoin") != std::string::npos) {
            pubkeyPrefix = {52};
            scriptPrefix = {13};
            segwitPrefix = "nc";
        } else {
            pubkeyPrefix = std::vector<unsigned char>(1,0);
            scriptPrefix = std::vector<unsigned char>(1,5);
            segwitPrefix = "bc";
        }
    }
    
}
