//
//  chain_configuration.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/20/18.
//
//

#include "chain_configuration.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace blocksci {
    
    void to_json(json& j, const ChainConfiguration& p) {
        j = json{
            {"coinName", p.coinName},
            {"dataDirectory", p.dataDirectory.str()},
            {"pubkeyPrefix", p.pubkeyPrefix},
            {"scriptPrefix", p.scriptPrefix},
            {"segwitPrefix", p.segwitPrefix},
            {"segwitActivationHeight", p.segwitActivationHeight}
        };
    }
    
    void from_json(const json& j, ChainConfiguration& p) {
        j.at("coinName").get_to(p.coinName);
        std::string dataDir;
        j.at("dataDirectory").get_to(dataDir);
        p.dataDirectory = dataDir;
        j.at("pubkeyPrefix").get_to(p.pubkeyPrefix);
        j.at("scriptPrefix").get_to(p.scriptPrefix);
        j.at("segwitPrefix").get_to(p.segwitPrefix);
        j.at("segwitActivationHeight").get_to(p.segwitActivationHeight);
    }
    
    void to_json(json& j, const ChainRPCConfiguration& p) {
        j = json{{"username", p.username}, {"password", p.password}, {"address", p.address}, {"port", p.port}};
    }
    
    void from_json(const json& j, ChainRPCConfiguration& p) {
        j.at("username").get_to(p.username);
        j.at("password").get_to(p.password);
        j.at("address").get_to(p.address);
        j.at("port").get_to(p.port);
    }
    
    ChainConfiguration ChainConfiguration::dash(const std::string &chainDir) {
        return {
            "dash",
            chainDir,
            {76},
            {16},
            "NONE",
            std::numeric_limits<BlockHeight>::max() // No segwit support
        };
    }
    
    ChainConfiguration ChainConfiguration::dashTestnet(const std::string &chainDir) {
        return {
            "dash_testnet",
            chainDir,
            {140},
            {19},
            "NONE",
            std::numeric_limits<BlockHeight>::max() // No segwit support
        };
    }
    
    ChainConfiguration ChainConfiguration::litecoin(const std::string &chainDir) {
        return {
            "litecoin",
            chainDir,
            {48},
            {50},
            "ltc",
            1201536
        };
    }
    
    ChainConfiguration ChainConfiguration::litecoinTestnet(const std::string &chainDir) {
        return {
            "litecoin_testnet",
            chainDir,
            {111},
            {58},
            "tltc",
            83278 // Guess based on block explorer
        };
    }
    
    ChainConfiguration ChainConfiguration::litecoinRegtest(const std::string &chainDir) {
        return {
            "litecoin_regtest",
            chainDir,
            {111},
            {58},
            "rltc",
            0
        };
    }
    
    ChainConfiguration ChainConfiguration::zcash(const std::string &chainDir) {
        return {
            "zcash",
            chainDir,
            {0x1C,0xB8},
            {0x1C,0xBD},
            "NONE",
            std::numeric_limits<BlockHeight>::max() // No segwit support
        };
    }
    
    ChainConfiguration ChainConfiguration::zcashTestnet(const std::string &chainDir) {
        return {
            "zcash_testnet",
            chainDir,
            {0x1D,0x25},
            {0x1C,0xBA},
            "NONE",
            std::numeric_limits<BlockHeight>::max() // No segwit support
        };
    }
    
    ChainConfiguration ChainConfiguration::namecoin(const std::string &chainDir) {
        return {
            "namecoin",
            chainDir,
            {52},
            {13},
            "nc",
            std::numeric_limits<BlockHeight>::max() // Not activated yet, but planned
        };
    }
    
    ChainConfiguration ChainConfiguration::namecoinTestnet(const std::string &chainDir) {
        return {
            "namecoin_testnet",
            chainDir,
            {111},
            {196},
            "tn",
            std::numeric_limits<BlockHeight>::max() // Not activated yet, but planned
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoinRegtest(const std::string &chainDir) {
        return {
            "bitcoin_regtest",
            chainDir,
            std::vector<unsigned char>(1,111),
            std::vector<unsigned char>(1,196),
            "bcrt",
            0
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoin(const std::string &chainDir) {
        return {
            "bitcoin",
            chainDir,
            std::vector<unsigned char>(1,0),
            std::vector<unsigned char>(1,5),
            "bc",
            481824
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoinTestnet(const std::string &chainDir) {
        return {
            "bitcoin_testnet",
            chainDir,
            std::vector<unsigned char>(1,111),
            std::vector<unsigned char>(1,196),
            "tb",
            834624
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoinCashRegtest(const std::string &chainDir) {
        return {
            "bitcoin_cash_regtest",
            chainDir,
            std::vector<unsigned char>(1,111),
            std::vector<unsigned char>(1,196),
            "NONE",
            std::numeric_limits<BlockHeight>::max() // No segwit support
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoinCash(const std::string &chainDir) {
        return {
            "bitcoin_cash",
            chainDir,
            std::vector<unsigned char>(1,0),
            std::vector<unsigned char>(1,5),
            "NONE",
            std::numeric_limits<BlockHeight>::max() // No segwit support
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoinCashTestnet(const std::string &chainDir) {
        return {
            "bitcoin_cash_testnet",
            chainDir,
            std::vector<unsigned char>(1,111),
            std::vector<unsigned char>(1,196),
            "NONE",
            std::numeric_limits<BlockHeight>::max() // No segwit support
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::bitcoin(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            8332
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::bitcoinTestnet(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            18332
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::bitcoinCash(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            8332
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::bitcoinCashTestnet(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            18332
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::litecoin(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            9332
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::litecoinTestnet(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            19332
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::dash(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            9998
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::dashTestnet(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            19998
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::namecoin(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            8336
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::namecoinTestnet(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            18336
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::zcash(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            8232
        };
    }
    
    ChainRPCConfiguration ChainRPCConfiguration::zcashTestnet(const std::string &username, const std::string &password) {
        return {
            username,
            password,
            "127.0.0.1",
            18232
        };
    }
}
