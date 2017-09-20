//
//  data_configuration.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "data_configuration.hpp"

#include <boost/filesystem.hpp>

namespace blocksci {
    
    void createDirectory(boost::filesystem::path dir);
    
    
    void createDirectory(boost::filesystem::path dir) {
        if(!(boost::filesystem::exists(dir))){
            boost::filesystem::create_directory(dir);
        }
    }
    
    DataConfiguration::DataConfiguration(const boost::filesystem::path &dataDirectory_) : dataDirectory(dataDirectory_) {
        auto dataDirectoryString = dataDirectory.native();
        if(dataDirectoryString.find("dash") != std::string::npos) {
            pubkeyPrefix = {76};
            scriptPrefix = {16};
        } else if(dataDirectoryString.find("litecoin") != std::string::npos) {
            pubkeyPrefix = {48};
            scriptPrefix = {50};
        } else if(dataDirectoryString.find("zcash") != std::string::npos) {
            pubkeyPrefix = {28,184};
            scriptPrefix = {28,189};
        } else if(dataDirectoryString.find("namecoin") != std::string::npos) {
            pubkeyPrefix = {52};
            scriptPrefix = {13};
        } else {
            pubkeyPrefix = std::vector<unsigned char>(1,0);
            scriptPrefix = std::vector<unsigned char>(1,5);
        }
        
        createDirectory(dataDirectory);
        createDirectory(scriptsDirectory());
        createDirectory(firstSeenDirectory());
        createDirectory(chainDirectory());
    }
    
}
