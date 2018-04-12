//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/util/file_writer.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/index/mempool_index.hpp>

#include <bitcoinapi/bitcoinapi.h>

#include <clipp.h>

#include <range/v3/range_for.hpp>

#include <chrono>
#include <iostream>
#include <future>
#include <unordered_map>

#include <csignal>

using namespace blocksci;

static volatile sig_atomic_t done = 0;

void term(int)
{
    done = 1;
}

using MempoolMap = std::unordered_map<blocksci::uint256, MempoolRecord, std::hash<blocksci::uint256>>;

boost::filesystem::path initializeRecordingFile(Blockchain &chain) {
    auto mostRecentBlock = chain[chain.size() - 1];
    auto &config = chain.getAccess().config;
    blocksci::FixedSizeFileWriter<uint32_t> indexFile(chain.getAccess().config.mempoolDirectory()/"index");
    auto fileNum = indexFile.size();
    indexFile.write(mostRecentBlock.endTxIndex());
    return config.mempoolDirectory()/std::to_string(fileNum);
}

class MempoolRecorder {
    blocksci::Blockchain chain;
    blocksci::BlockHeight lastHeight;
    MempoolMap mempool;
    BitcoinAPI &bitcoinAPI;
    blocksci::FixedSizeFileWriter<MempoolRecord> txTimeFile;
    
public:
    MempoolRecorder(const std::string &dataLocation, BitcoinAPI &bitcoinAPI_) :
    chain(dataLocation),
    lastHeight(chain.size()),
    bitcoinAPI(bitcoinAPI_),
    txTimeFile(initializeRecordingFile(chain)) {
        auto rawMempool = bitcoinAPI.getrawmempool();
        for (auto &txHashString : rawMempool) {
            auto txHash = uint256S(txHashString);
            auto it = mempool.find(txHash);
            if (it == mempool.end()) {
                mempool[txHash] = {0};
            }
        }
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
                mempool[txHash] = {time};
            }
        }
    }
    
    void recordMempool() {
        chain.reload();
        auto blockCount = static_cast<BlockHeight>(chain.size());
        for (; lastHeight < blockCount; lastHeight++) {
            RANGES_FOR(auto tx, chain[lastHeight]) {
                auto it = mempool.find(tx.getHash());
                if (it != mempool.end()) {
                    auto &txData = it->second;
                    txTimeFile.write(txData);
                    mempool.erase(it);
                } else {
                    txTimeFile.write({0});
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
            if (it->second.time < clearTime) {
                it = mempool.erase(it);
            } else {
                ++it;
            }
        }
    }
};


int main(int argc, char * argv[]) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    
    
    std::string username;
    std::string password;
    std::string address = "127.0.0.1";
    int port = 9998;
    auto rpcOptions = (
        (clipp::required("--username") & clipp::value("username", username)) % "RPC username",
        (clipp::required("--password") & clipp::value("password", password)) % "RPC password",
        (clipp::option("--address") & clipp::value("address", address)) % "RPC address",
        (clipp::option("--port") & clipp::value("port", port)) % "RPC port"
    ).doc("RPC options");

    std::string dataLocation;
    auto cli = (clipp::value("data location", dataLocation), rpcOptions);
    auto res = parse(argc, argv, cli);
    if (res.any_error()) {
        std::cout << "Invalid command line parameter\n" << clipp::make_man_page(cli, argv[0]);
        return 0;
    }

    BitcoinAPI bitcoinAPI{username, password, address, port};
    
    MempoolRecorder recorder{dataLocation, bitcoinAPI};
    
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
