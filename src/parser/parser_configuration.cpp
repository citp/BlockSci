//
//  parser_configuration.h
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "parser_configuration.hpp"
#include <blocksci/hash.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <boost/filesystem.hpp>

ParserConfiguration::ParserConfiguration() : DataConfiguration() {}

ParserConfiguration::ParserConfiguration(const boost::filesystem::path &dataDirectory_) : DataConfiguration(dataDirectory_) {
    boost::filesystem::path dir(parserDirectory());
    
    if(!(boost::filesystem::exists(dir))){
        boost::filesystem::create_directory(dir);
    }
}

#ifdef BLOCKSCI_FILE_PARSER
FileParserConfiguration::FileParserConfiguration() : ParserConfiguration() {

}

FileParserConfiguration::FileParserConfiguration(const boost::filesystem::path &bitcoinDirectory_, const boost::filesystem::path &dataDirectory_) : ParserConfiguration(dataDirectory_), bitcoinDirectory(bitcoinDirectory_) {
    auto bitcoinDirectoryString = bitcoinDirectory.native();
    if (bitcoinDirectoryString.find("litecoin") != std::string::npos || bitcoinDirectoryString.find("Litecoin") != std::string::npos) {
        blockMagic = 0xdbb6c0fb;
        workHashFunction = doubleSha256;
    } else if (bitcoinDirectoryString.find("regtest") != std::string::npos) {
        blockMagic = 0xdab5bffa;
        workHashFunction = doubleSha256;
    } else {
        blockMagic = 0xd9b4bef9;
        workHashFunction = doubleSha256;
    }
    
    
}

boost::filesystem::path FileParserConfiguration::pathForBlockFile(int fileNum) const {
    std::ostringstream convert;
    convert << fileNum;
    std::string numString = convert.str();
    return (bitcoinDirectory/"blocks"/"blk").concat(std::string(5 - numString.size(), '0')).concat(numString).concat(".dat");
}
#endif

#ifdef BLOCKSCI_RPC_PARSER
RPCParserConfiguration::RPCParserConfiguration() : ParserConfiguration() {}

RPCParserConfiguration::RPCParserConfiguration(std::string username_, std::string password_, std::string address_, int port_, const boost::filesystem::path &dataDirectory_) : ParserConfiguration(dataDirectory_), username(username_), password(password_), address(address_), port(port_) {}


BitcoinAPI RPCParserConfiguration::createBitcoinAPI() const {
    return BitcoinAPI{username, password, address, port};
}
#endif

