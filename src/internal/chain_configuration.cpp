//
//  chain_configuration.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/20/18.
//
//

#include "chain_configuration.hpp"

#include <simpleini/SimpleIni.h>

namespace blocksci {
    
    ChainConfiguration ChainConfiguration::dash() {
        return {
            {76},
            {16},
            "NONE"
        };
    }
    
    ChainConfiguration ChainConfiguration::dashTestNet() {
        return {
            {140},
            {19},
            "NONE"
        };
    }
    
    ChainConfiguration ChainConfiguration::litecoin() {
        return {
            {48},
            {50},
            "ltc"
        };
    }
    
    ChainConfiguration ChainConfiguration::zcash() {
        return {
            {28,184},
            {28,189},
            "NONE"
        };
    }
    
    ChainConfiguration ChainConfiguration::namecoin() {
        return {
            {52},
            {13},
            "nc"
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoinRegtest() {
        return {
            std::vector<unsigned char>(1,111),
            std::vector<unsigned char>(1,196),
            "bcrt"
        };
    }
    
    ChainConfiguration ChainConfiguration::bitcoin() {
        return {
            std::vector<unsigned char>(1,0),
            std::vector<unsigned char>(1,5),
            "bc"
        };
    }
}
