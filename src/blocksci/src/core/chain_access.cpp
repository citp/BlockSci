//
//  chain_access.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/22/18.
//

#include <blocksci/core/chain_access.hpp>
#include <blocksci/util/data_configuration.hpp>

namespace blocksci {
    ChainAccess::ChainAccess(const DataConfiguration &config) :
        blockFile(config.blockFilePath()),
        blockCoinbaseFile(config.blockCoinbaseFilePath()),
        txFile(config.txFilePath()),
        sequenceFile(config.sequenceFilePath()),
        txHashesFile(config.txHashesFilePath()),
        blocksIgnored(config.blocksIgnored),
        errorOnReorg(config.errorOnReorg) {
            setup();
        }
} // namespace blocksci
