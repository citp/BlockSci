//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/cluster/cluster_manager.hpp>
#include <blocksci/heuristics/change_address.hpp>

#include <clipp.h>

#include <iostream>

int main(int argc, char * argv[]) {
    std::string configLocation;
    std::string outputLocation;
    bool overwrite;
    auto cli = (
                clipp::value("config file location", configLocation),
                clipp::value("output location", outputLocation),
                clipp::option("--overwrite").set(overwrite).doc("Overwrite existing cluster files if they exist")
    );
    auto res = parse(argc, argv, cli);
    if (res.any_error()) {
        std::cout << "Invalid command line parameter\n" << clipp::make_man_page(cli, argv[0]);
        return 0;
    }
    
    blocksci::Blockchain chain(configLocation);
    
    blocksci::ClusterManager::createClustering(chain, blocksci::heuristics::NoChange{}, outputLocation, overwrite);
    return 0;
}
