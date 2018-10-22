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
    ParserConfigurationBase(const blocksci::DataConfiguration &config);
    
    filesystem::path parserDirectory() const {
        return filesystem::path{dataConfig.chainConfig.dataDirectory}/"parser";
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
};

#ifdef BLOCKSCI_FILE_PARSER
struct ChainDiskConfiguration {
    filesystem::path coinDirectory;
    uint32_t blockMagic;
    std::string hashFuncName;
    std::function<blocksci::uint256(const char *data, unsigned long len)> workHashFunction;
    
    ChainDiskConfiguration() {}
    ChainDiskConfiguration(const std::string bitcoinDir, uint32_t blockMagic_, std::string hashFuncName) : coinDirectory(bitcoinDir), blockMagic(blockMagic_), hashFuncName(std::move(hashFuncName)) {
        resetHashFunc();
    }
    
    void resetHashFunc();
    
    static ChainDiskConfiguration litecoin(const std::string &path);
    static ChainDiskConfiguration litecoinTestnet(const std::string &path);
    static ChainDiskConfiguration litecoinRegtest(const std::string &path);
    
    static ChainDiskConfiguration bitcoinRegtest(const std::string &path);
    static ChainDiskConfiguration bitcoinTestnet(const std::string &path);
    static ChainDiskConfiguration bitcoin(const std::string &path);
    
    static ChainDiskConfiguration bitcoinCashRegtest(const std::string &path);
    static ChainDiskConfiguration bitcoinCashTestnet(const std::string &path);
    static ChainDiskConfiguration bitcoinCash(const std::string &path);
};

void to_json(nlohmann::json& j, const ChainDiskConfiguration& p);
void from_json(const nlohmann::json& j, ChainDiskConfiguration& p);

template<>
struct ParserConfiguration<FileTag> : public ParserConfigurationBase {
    ParserConfiguration();
    ParserConfiguration(const blocksci::DataConfiguration &dataConfig, const ChainDiskConfiguration &diskConfig);
    
    ChainDiskConfiguration diskConfig;
    
    filesystem::path pathForBlockFile(int fileNum) const;
};

#endif

#ifdef BLOCKSCI_RPC_PARSER

class BitcoinAPI;


struct ChainRPCConfiguration {
    std::string username;
    std::string password;
    std::string address;
    int port = 0;
    
    static ChainRPCConfiguration bitcoin(const std::string &username, const std::string &password);
    static ChainRPCConfiguration bitcoinTestnet(const std::string &username, const std::string &password);
    
    static ChainRPCConfiguration bitcoinCash(const std::string &username, const std::string &password);
    static ChainRPCConfiguration bitcoinCashTestnet(const std::string &username, const std::string &password);
    
    static ChainRPCConfiguration litecoin(const std::string &username, const std::string &password);
    static ChainRPCConfiguration litecoinTestnet(const std::string &username, const std::string &password);
    
    static ChainRPCConfiguration namecoin(const std::string &username, const std::string &password);
    static ChainRPCConfiguration namecoinTestnet(const std::string &username, const std::string &password);
    
    static ChainRPCConfiguration dash(const std::string &username, const std::string &password);
    static ChainRPCConfiguration dashTestnet(const std::string &username, const std::string &password);
    
    static ChainRPCConfiguration zcash(const std::string &username, const std::string &password);
    static ChainRPCConfiguration zcashTestnet(const std::string &username, const std::string &password);
};

void to_json(nlohmann::json& j, const ChainRPCConfiguration& p);
void from_json(const nlohmann::json& j, ChainRPCConfiguration& p);

template<>
struct ParserConfiguration<RPCTag> : public ParserConfigurationBase {
    ParserConfiguration();
    ParserConfiguration(const blocksci::DataConfiguration &dataConfig, const ChainRPCConfiguration &rpc);
    
    ChainRPCConfiguration config;
    
    BitcoinAPI createBitcoinAPI() const;
};
#endif


#endif /* parser_configuration_h */
