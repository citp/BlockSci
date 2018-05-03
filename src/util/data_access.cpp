//
//  data_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/24/18.
//
//

#include <blocksci/util/data_access.hpp>

#include <blocksci/chain/chain_access.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/index/mempool_index.hpp>

namespace blocksci {
    
    DataAccess::DataAccess() = default;

    DataAccess::DataAccess(DataConfiguration config_) :
    config(std::move(config_)),
    chain{std::make_unique<ChainAccess>(config.chainDirectory(), config.blocksIgnored, config.errorOnReorg)},
    scripts{std::make_unique<ScriptAccess>(config.scriptsDirectory())},
    addressIndex{std::make_unique<AddressIndex>(config.addressDBFilePath(), true)},
    hashIndex{std::make_unique<HashIndex>(config.hashIndexFilePath(), true)},
    mempoolIndex{std::make_unique<MempoolIndex>(config.mempoolDirectory())} {}
    
    DataAccess::DataAccess(DataAccess &&) = default;
    DataAccess &DataAccess::operator=(DataAccess &&) = default;
    DataAccess::~DataAccess() = default;

    void DataAccess::reload() {
        chain->reload();
        scripts->reload();
        mempoolIndex->reload();
    }
}
