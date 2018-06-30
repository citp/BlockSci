//
//  parser_configuration.h
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef parser_configuration_h
#define parser_configuration_h

#include "config.hpp"

#include <blocksci/bitcoin_uint256.hpp>
#include <blocksci/data_configuration.hpp>

#include <boost/filesystem.hpp>

#include <functional>

struct ParserConfiguration : public blocksci::DataConfiguration {
    ParserConfiguration();
    ParserConfiguration(const boost::filesystem::path &dataDirectory_);
    
    boost::filesystem::path parserDirectory() const {
        return dataDirectory/"parser";
    }
    
    boost::filesystem::path addressCacheFile() const {
        return parserDirectory()/"addressCache.dat";
    }
    
    boost::filesystem::path addressBloomCacheFile() const {
        return parserDirectory()/"addressBloomCache.dat";
    }
    
    boost::filesystem::path utxoDBPath() const {
        return parserDirectory()/"utxoDB";
    }
    
    boost::filesystem::path blockListPath() const {
        return parserDirectory()/"blockList.dat";
    }
    
    boost::filesystem::path txUpdatesFilePath() const {
        return parserDirectory()/"txUpdates";
    }
};

#ifdef BLOCKSCI_FILE_PARSER
struct FileParserConfiguration : public ParserConfiguration {
    FileParserConfiguration();
    FileParserConfiguration(const boost::filesystem::path &bitcoinDirectory_, const boost::filesystem::path &dataDirectory_);
    
    boost::filesystem::path bitcoinDirectory;
    uint32_t blockMagic;
    std::function<blocksci::uint256(const char *data, unsigned long len)> workHashFunction;
    
    
    boost::filesystem::path pathForBlockFile(int fileNum) const;
};
#endif

#ifdef BLOCKSCI_RPC_PARSER

class BitcoinAPI;

struct RPCParserConfiguration : public ParserConfiguration {
    RPCParserConfiguration();
    RPCParserConfiguration(std::string username, std::string password, std::string address, int port, const boost::filesystem::path &dataDirectory_);
    
    std::string username;
    std::string password;
    std::string address;
    int port;
    
    BitcoinAPI createBitcoinAPI() const;
};
#endif


#endif /* parser_configuration_h */
