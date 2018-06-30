//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/blocksci.hpp>
#include <blocksci/data_configuration.hpp>
#include <blocksci/data_access.hpp>

#include <bitcoinapi/bitcoinapi.h>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time.hpp>

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <future>

#include <fstream>

using namespace blocksci;
// boost::posix_time::hours(24 * 5)
class MempoolRecorder {
    uint32_t lastHeight;
    std::unordered_map<uint256, time_t> mempool;
    const DataConfiguration &config;
    BitcoinAPI &bitcoinAPI;
    blocksci::FixedSizeFileMapper<time_t, boost::iostreams::mapped_file::readwrite> txTimeFile;
public:
    MempoolRecorder(const DataConfiguration &config_, BitcoinAPI &bitcoinAPI_) : config(config_), bitcoinAPI(bitcoinAPI_), txTimeFile(config.dataDirectory/"mempool") {
        blocksci::ChainAccess chain(config, false, 0);
        auto blocks = chain.getBlocks();
        lastHeight = blocks.size();
        
        if (txTimeFile.size() == 0) {
            // Record starting txNum in position 0
            auto mostRecentBlock = blocks.back();
            auto lastTx = mostRecentBlock.txes(chain).back();
            txTimeFile.write(lastTx.txNum + 1);
        } else {
            // Fill in 0 timestamp where data is missing
            auto firstNum = *txTimeFile.getData(0);
            auto mostRecentBlock = blocks.back();
            auto lastTx = mostRecentBlock.txes(chain).back();
            auto txesSinceStart = static_cast<size_t>(lastTx.txNum - firstNum);
            auto currentTxCount = txTimeFile.size() - 1;
            if (currentTxCount < txesSinceStart) {
                for (size_t i = currentTxCount; i < txesSinceStart; i++) {
                    txTimeFile.write(0);
                }
            }
        }
        
        auto rawMempool = bitcoinAPI.getrawmempool();
        for (auto &txHashString : rawMempool) {
            auto txHash = uint256S(txHashString);
            auto it = mempool.find(txHash);
            if (it == mempool.end()) {
                mempool[txHash] = 0;
            }
        }
    }
    
    void updateMempool() {
        time_t curTime;
        time(&curTime);
        auto rawMempool = bitcoinAPI.getrawmempool();
        for (auto &txHashString : rawMempool) {
            auto txHash = uint256S(txHashString);
            auto it = mempool.find(txHash);
            if (it == mempool.end()) {
                mempool[txHash] = curTime;
            }
        }
    }
    
    void recordMempool() {
        blocksci::ChainAccess chain(config, false, 0);
        auto blocks = chain.getBlocks();
        auto blockCount = blocks.size();
        for (uint32_t i = lastHeight; i < blockCount; i++) {
            auto &block = blocks[i];
            for (auto tx : block.txes(chain)) {
                auto it = mempool.find(tx.getHash(chain));
                if (it != mempool.end()) {
                    auto &txData = it->second;
                    txTimeFile.write(txData);
                    mempool.erase(it);
                } else {
                    txTimeFile.write(boost::posix_time::not_a_date_time);
                }
            }
        }
    }
    
    void clearOldMempool() {
        time_t clearTime;
        time(&clearTime);
        struct tm * ptm;
        ptm = gmtime(&clearTime);
        ptm->tm_mday -= 5;
        clearTime = mktime(ptm);
        auto it = mempool.begin();
        while (it != mempool.end()) {
            if (it->second < clearTime) {
                it = mempool.erase(it);
            } else {
                ++it;
            }
        }
    }
};


int main(int argc, const char * argv[]) {
    assert(argc == 2);
    
    std::string user = "daniel";
    std::string password = "thisisthepasswordformynode";
    std::string host = "127.0.0.1";
    int port = 8232;

    BitcoinAPI bitcoinAPI{user, password, host, port};
    
    DataConfiguration config(argv[1]);
    
    MempoolRecorder recorder{config, bitcoinAPI};
    
    int updateCount = 0;
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        recorder.updateMempool();
        updateCount++;
        if (updateCount % 5 == 0) {
            recorder.recordMempool();
        }
        if (updateCount % (4 * 60 * 60 * 24) == 0) {
            recorder.clearOldMempool();
            updateCount = 0;
        }
    }
    
    
    return 0;
}
