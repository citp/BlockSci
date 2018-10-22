//
//  chain_configuration.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/20/18.
//  Copyright © 2018 Harry Kalodner. All rights reserved.
//

#ifndef chain_configuration_h
#define chain_configuration_h

#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/core/typedefs.hpp>

#include <nlohmann/json_fwd.hpp>

#include <wjfilesystem/path.h>

#include <string>
#include <vector>

namespace blocksci {
    
    struct ChainConfiguration {
        std::string coinName;
        filesystem::path dataDirectory;
        
        std::vector<unsigned char> pubkeyPrefix;
        std::vector<unsigned char> scriptPrefix;
        
        std::string segwitPrefix;
        
        BlockHeight segwitActivationHeight;
        
        static ChainConfiguration bitcoin(const std::string &chainDir);
        static ChainConfiguration bitcoinTestnet(const std::string &chainDir);
        static ChainConfiguration bitcoinRegtest(const std::string &chainDir);
        
        static ChainConfiguration bitcoinCash(const std::string &chainDir);
        static ChainConfiguration bitcoinCashTestnet(const std::string &chainDir);
        static ChainConfiguration bitcoinCashRegtest(const std::string &chainDir);
        
        static ChainConfiguration litecoin(const std::string &chainDir);
        static ChainConfiguration litecoinTestnet(const std::string &chainDir);
        static ChainConfiguration litecoinRegtest(const std::string &chainDir);
        
        static ChainConfiguration namecoin(const std::string &chainDir);
        static ChainConfiguration namecoinTestnet(const std::string &chainDir);
        
        static ChainConfiguration dash(const std::string &chainDir);
        static ChainConfiguration dashTestnet(const std::string &chainDir);
        
        static ChainConfiguration zcash(const std::string &chainDir);
        static ChainConfiguration zcashTestnet(const std::string &chainDir);
        
    };
    
    void to_json(nlohmann::json& j, const ChainConfiguration& p);
    void from_json(const nlohmann::json& j, ChainConfiguration& p);
}

#endif /* chain_configuration_h */
