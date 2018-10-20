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

#include <string>
#include <vector>

namespace blocksci {
    
    struct ChainConfiguration {
        std::vector<unsigned char> pubkeyPrefix;
        std::vector<unsigned char> scriptPrefix;
        
        std::string segwitPrefix;
        
        static ChainConfiguration bitcoin();
        static ChainConfiguration bitcoinRegtest();
        
        static ChainConfiguration litecoin();
        
        static ChainConfiguration zcash();
        
        static ChainConfiguration namecoin();
        
        static ChainConfiguration dash();
        static ChainConfiguration dashTestNet();
        
    };
}

#endif /* chain_configuration_h */
