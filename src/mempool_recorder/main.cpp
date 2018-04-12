//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/util/data_access.hpp>
#include <blocksci/util/file_writer.hpp>
#include <blocksci/chain/block.hpp>

#include <bitcoinapi/bitcoinapi.h>

#include <range/v3/range_for.hpp>

#include <chrono>
#include <iostream>
#include <future>
#include <unordered_map>

#include <csignal>

static volatile sig_atomic_t done = 0;

void term(int)
{
    done = 1;
}

using namespace blocksci;
class MempoolRecorder {
    blocksci::BlockHeight lastHeight;
    std::unordered_map<uint256, time_t, std::hash<blocksci::uint256>> mempool;
    const DataConfiguration &config;
    BitcoinAPI &bitcoinAPI;
    blocksci::FixedSizeFileWriter<time_t> txTimeFile;
public:
    MempoolRecorder(const DataConfiguration &config_, BitcoinAPI &bitcoinAPI_) : config(config_), bitcoinAPI(bitcoinAPI_), txTimeFile(config.dataDirectory/"mempool") {
        blocksci::ChainAccess chain(config);
        lastHeight = chain.blockCount();
        
       
        auto mostRecentBlock = chain.getBlock(lastHeight - 1);
        auto lastTxIndex = mostRecentBlock->firstTxIndex + mostRecentBlock->numTxes - 1;
        if (txTimeFile.size() == 0) {
            // Record starting txNum in position 0
            txTimeFile.write(lastTxIndex + 1);
        } else {
            // Fill in 0 timestamp where data is missing
            auto firstNum = txTimeFile.read(0);
            auto txesSinceStart = static_cast<size_t>(lastTxIndex - firstNum);
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
        txTimeFile.flush();
    }
    
    void updateMempool() {
        using namespace std::chrono;
        system_clock::time_point tp = system_clock::now();
        auto time = system_clock::to_time_t(tp);
        auto rawMempool = bitcoinAPI.getrawmempool();
        for (auto &txHashString : rawMempool) {
            auto txHash = uint256S(txHashString);
            auto it = mempool.find(txHash);
            if (it == mempool.end()) {
                mempool[txHash] = time;
            }
        }
    }
    
    void recordMempool() {
        blocksci::DataAccess access(config);
        auto blockCount = access.chain.blockCount();
        for (; lastHeight < blockCount; lastHeight++) {
            auto block = Block(lastHeight, access);
            RANGES_FOR(auto tx, block) {
                auto it = mempool.find(tx.getHash());
                if (it != mempool.end()) {
                    auto &txData = it->second;
                    txTimeFile.write(txData);
                    mempool.erase(it);
                } else {
                    txTimeFile.write(0);
                }
            }
        }
        txTimeFile.flush();
    }
    
    void clearOldMempool() {
        using namespace std::chrono;
        system_clock::time_point tp = system_clock::now();
        tp -= std::chrono::hours(5 * 24);
        auto clearTime = system_clock::to_time_t(tp);
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
    
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    
    std::string user = "blocksci";
    std::string password = "blocksci";
    std::string host = "localhost";
    int port = 8331;

    BitcoinAPI bitcoinAPI{user, password, host, port};
    
    DataConfiguration config(argv[1], false, 0);
    
    MempoolRecorder recorder{config, bitcoinAPI};
    
    int updateCount = 0;
    while(!done) {
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
    
    std::cout << "Shut down mempool recorder\n";
}
