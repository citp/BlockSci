//
//  blockchain.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/address/address.hpp>

#include <internal/address_info.hpp>
#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>
#include <internal/address_output_range.hpp>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/operations.hpp>
#include <range/v3/range_for.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/transform.hpp>

namespace blocksci {
    
    Blockchain::Blockchain(std::unique_ptr<DataAccess> access_) : BlockRange{{0, access_->getChain().blockCount()}, access_.get()}, access(std::move(access_)) {}
    
    Blockchain::Blockchain(const DataConfiguration &config) : Blockchain(std::make_unique<DataAccess>(config)) {}
    
    Blockchain::Blockchain(const std::string &configPath, BlockHeight maxBlock) : Blockchain(loadBlockchainConfig(configPath, true, maxBlock)) {}
    
    Blockchain::Blockchain(const std::string &configPath) : Blockchain(configPath, BlockHeight{0}) {}
    
    
    Blockchain::~Blockchain() = default;
    
    std::string Blockchain::dataLocation() const {
        return access->config.chainConfig.dataDirectory.str();
    }
    
    std::string Blockchain::configLocation() const {
        return access->config.configPath;
    }
    
    void Blockchain::reload() {
        access->reload();
        sl.stop = access->getChain().blockCount();
    }

    bool Blockchain::isParserRunning() {
        return access->config.pidFilePath().exists();
    }
    
    uint32_t txCount(Blockchain &chain) {
        auto lastBlock = chain[static_cast<int>(chain.size()) - BlockHeight{1}];
        return lastBlock.endTxIndex();
    }
    
    uint32_t Blockchain::addressCount(AddressType::Enum type) const {
        return access->getScripts().scriptCount(dedupType(type));
    }
} // namespace blocksci
