//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/blocksci.hpp>
#include <libcluster/cluster_manager.hpp>
#include <libcluster/cluster.hpp>

using namespace blocksci;


int main(int argc, const char * argv[]) {
    assert(argc == 2);
    
    Blockchain chain(argv[1]);
    ClusterManager manager(argv[2]);
    
    std::vector<uint32_t> sizes;
    sizes.reserve(manager.clusterCount());
    for (auto cluster : manager.getClusters()) {
        auto size = cluster.getAddressCount(ScriptType::Enum::PUBKEYHASH) + cluster.getAddressCount(ScriptType::Enum::SCRIPTHASH);
        std::cout << size << "\n";
    }
    
    return 0;
}
