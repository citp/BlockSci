//
//  parser_configuration.h
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "parser_configuration.hpp"
#include <internal/hash.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <wjfilesystem/path.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

ParserConfigurationBase::ParserConfigurationBase() : dataConfig() {}

ParserConfigurationBase::ParserConfigurationBase(const blocksci::DataConfiguration &config) : dataConfig(config) {
    filesystem::path dir(parserDirectory());
    
    if (!dir.exists()){
        filesystem::create_directory(dir);
    }
    
    if (addressPath().exists()) {
        filesystem::create_directory(addressPath());
    }
    
    if (!utxoAddressStatePath().exists()){
        filesystem::create_directory(utxoAddressStatePath());
    }
    
    if (!utxoAddressStatePath().exists()){
        filesystem::create_directory(utxoAddressStatePath());
    }
    
    if (!addressPath().exists()){
        filesystem::create_directory(addressPath());
    }
    
    
}

#ifdef BLOCKSCI_FILE_PARSER
ParserConfiguration<FileTag>::ParserConfiguration() : ParserConfigurationBase() {}

ParserConfiguration<FileTag>::ParserConfiguration(const blocksci::DataConfiguration &dataConfig, const ChainDiskConfiguration &diskConfig) : ParserConfigurationBase(dataConfig), diskConfig(diskConfig) {}

void ChainDiskConfiguration::resetHashFunc() {
    if (hashFuncName == "doubleSha256") {
        workHashFunction = doubleSha256;
    } else {
        throw std::runtime_error("Unsupported hash func type");
    }
}

void to_json(json& j, const ChainDiskConfiguration& p) {
    j = json{{"blockMagic", p.blockMagic}, {"hashFuncName", p.hashFuncName}, {"coinDirectory", p.coinDirectory.str()}};
}

void from_json(const json& j, ChainDiskConfiguration& p) {
    j.at("blockMagic").get_to(p.blockMagic);
    j.at("hashFuncName").get_to(p.hashFuncName);
    std::string coinDirString;
    j.at("coinDirectory").get_to(coinDirString);
    p.coinDirectory = coinDirString;
    
    p.resetHashFunc();
}

filesystem::path ParserConfiguration<FileTag>::pathForBlockFile(int fileNum) const {
    std::ostringstream convert;
    convert << fileNum;
    std::string numString = convert.str();
    std::string filename = "blk" + std::string(5 - numString.size(), '0') + numString + ".dat";
    return diskConfig.coinDirectory/"blocks"/filename;
}

ChainDiskConfiguration ChainDiskConfiguration::litecoin(const std::string &path) {
    return {
        path,
        0xdbb6c0fb,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::litecoinTestnet(const std::string &path) {
    return {
        path,
        0xf1c8d2fd,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::litecoinRegtest(const std::string &path) {
    return {
        path,
        0xdab5bffa,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::bitcoinRegtest(const std::string &path) {
    return {
        path,
        0xdab5bffa,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::bitcoinTestnet(const std::string &path) {
    return {
        path,
        0x0709110b,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::bitcoin(const std::string &path) {
    return {
        path,
        0xd9b4bef9,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::bitcoinCashRegtest(const std::string &path) {
    return {
        path,
        0xdab5bffa,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::bitcoinCashTestnet(const std::string &path) {
    return {
        path,
        0x0709110b,
        "doubleSha256"
    };
}

ChainDiskConfiguration ChainDiskConfiguration::bitcoinCash(const std::string &path) {
    return {
        path,
        0xd9b4bef9,
        "doubleSha256"
    };
}
#endif

#ifdef BLOCKSCI_RPC_PARSER

ParserConfiguration<RPCTag>::ParserConfiguration() : ParserConfigurationBase() {}

ParserConfiguration<RPCTag>::ParserConfiguration(const blocksci::DataConfiguration &dataConfig, const blocksci::ChainRPCConfiguration &rpc) : ParserConfigurationBase(dataConfig), config(rpc) {}


BitcoinAPI ParserConfiguration<RPCTag>::createBitcoinAPI() const {
    return BitcoinAPI{config.username, config.password, config.address, config.port};
}

#endif

