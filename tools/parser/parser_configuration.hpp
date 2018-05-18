//
//  parser_configuration.h
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef parser_configuration_h
#define parser_configuration_h

#include "parser_fwd.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>

#include <internal/data_configuration.hpp>

#include <wjfilesystem/path.h>

#include <functional>

struct ParserConfigurationBase {
    blocksci::DataConfiguration dataConfig;
    
    ParserConfigurationBase();
    ParserConfigurationBase(const std::string &dataDirectory_);
    
    filesystem::path parserDirectory() const {
        return filesystem::path{dataConfig.dataDirectory}/"parser";
    }
    
    filesystem::path utxoCacheFile() const {
        return parserDirectory()/"utxoCache.dat";
    }
    
    filesystem::path utxoAddressStatePath() const {
        return parserDirectory()/"utxoAddressState";
    }
    
    filesystem::path utxoScriptStatePath() const {
        return parserDirectory()/"utxoScriptState";
    }
    
    filesystem::path addressPath() const {
        return parserDirectory()/"address";
    }
    
    filesystem::path blockListPath() const {
        return parserDirectory()/"blockList.dat";
    }
    
    std::string txUpdatesFilePath() const {
        return (parserDirectory()/"txUpdates").str();
    }
    
    bool witnessActivatedAtHeight(uint32_t blockHeight) const;
};

#ifdef BLOCKSCI_FILE_PARSER
template<>
struct ParserConfiguration<FileTag> : public ParserConfigurationBase {
    ParserConfiguration();
    ParserConfiguration(filesystem::path bitcoinDirectory_, const std::string &dataDirectory_);
    
    filesystem::path bitcoinDirectory;
    uint32_t blockMagic = 0;
    std::function<blocksci::uint256(const char *data, unsigned long len)> workHashFunction;
    
    
    filesystem::path pathForBlockFile(int fileNum) const;
};
#endif

#ifdef BLOCKSCI_RPC_PARSER

class BitcoinAPI;

template<>
struct ParserConfiguration<RPCTag> : public ParserConfigurationBase {
    ParserConfiguration();
    ParserConfiguration(std::string username, std::string password, std::string address, int port, const std::string &dataDirectory_);
    
    std::string username;
    std::string password;
    std::string address;
    int port = 0;
    
    BitcoinAPI createBitcoinAPI() const;
};
#endif


#endif /* parser_configuration_h */
