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
#include <range/v3/front.hpp>
#include <range/v3/range_for.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/transform.hpp>

namespace blocksci {
    
    Blockchain::Blockchain(std::unique_ptr<DataAccess> access_) : BlockRange{{0, access_->getChain().blockCount()}, access_.get()}, access(std::move(access_)) {}
    
    Blockchain::Blockchain(const DataConfiguration &config) : Blockchain(std::make_unique<DataAccess>(config)) {}
    
    Blockchain::Blockchain(const std::string &dataDirectory, BlockHeight maxBlock) : Blockchain(loadBlockchainConfig(dataDirectory, true, maxBlock)) {}
    
    Blockchain::Blockchain(const std::string &dataDirectory) : Blockchain(dataDirectory, BlockHeight{0}) {}
    
    
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
    
    uint32_t txCount(Blockchain &chain) {
        auto lastBlock = chain[static_cast<int>(chain.size()) - BlockHeight{1}];
        return lastBlock.endTxIndex();
    }
    
    uint32_t Blockchain::addressCount(AddressType::Enum type) const {
        return access->getScripts().scriptCount(dedupType(type));
    }
    
    std::map<int64_t, Address> mostValuableAddresses(Blockchain &chain) {
        RawAddressOutputRange range{*chain.getAccess().addressIndex};
        auto grouped = range | ranges::view::group_by([](auto pair1, auto pair2) { return pair1.first == pair2.first; });
        std::map<int64_t, Address> topAddresses;
        
        RANGES_FOR(auto outputGroup, grouped) {
            auto address = ranges::front(outputGroup).first;
            auto balancesIfUnspent = outputGroup | ranges::view::transform([&](auto pair) -> int64_t {
                Output out{OutputPointer{pair.second.txNum, pair.second.inoutNum}, chain.getAccess()};
                return out.isSpent() ? 0 : out.getValue();
            });
            
            int64_t balance = ranges::accumulate(balancesIfUnspent, int64_t{0});
            if (topAddresses.size() < 100) {
                topAddresses.insert(std::make_pair(balance, Address{address, chain.getAccess()}));
            } else {
                auto lowestVal = topAddresses.begin();
                if (balance > lowestVal->first) {
                    topAddresses.erase(lowestVal);
                    topAddresses.insert(std::make_pair(balance, Address{address, chain.getAccess()}));
                }
            }
        }
        return topAddresses;
    }
} // namespace blocksci
