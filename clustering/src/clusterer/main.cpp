//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "dset/dset.h"

#include <blocksci/blocksci.hpp>
#include <blocksci/data_access.hpp>
#include <blocksci/script.hpp>
#include <blocksci/scripts/script_pointer.hpp>

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <future>

#include <fstream>

using namespace blocksci;

std::vector<std::pair<ScriptPointer, ScriptPointer>> process_transaction(const Transaction &tx) {
    std::vector<std::pair<ScriptPointer, ScriptPointer>> pairsToUnion;
    
    if (!isCoinjoin(tx) && !isCoinbase(tx)) {
        auto firstAddress = tx.inputs()[0].getAddress();
        for (uint16_t i = 1; i < tx.inputCount(); i++) {
            pairsToUnion.emplace_back(firstAddress, tx.inputs()[i].getAddress());
        }
        
        if (auto change = getChangeOutput(tx)) {
            pairsToUnion.emplace_back(change->getAddress(), firstAddress);
        }
    }
    return pairsToUnion;
}

void link_addresses(const ScriptPointer &address1, const ScriptPointer &address2, DisjointSets &sets, const std::unordered_map<ScriptType::Enum, uint32_t> &addressStarts) {
    auto firstAddressIndex = addressStarts.at(address1.type) + address1.scriptNum;
    auto secondAddressIndex = addressStarts.at(address2.type) + address2.scriptNum;;
    sets.unite(firstAddressIndex, secondAddressIndex);
}

template <typename Job>
void segmentWork(uint32_t start, uint32_t end, uint32_t segmentCount, Job job) {
    uint32_t total = end - start;
    auto segmentSize = total / segmentCount;
    auto segmentsRemaining = total % segmentCount;
    std::vector<std::pair<uint32_t, uint32_t>> segments;
    uint32_t i = 0;
    while(i < total) {
        uint32_t startSegment = i;
        i += segmentSize;
        if (segmentsRemaining > 0) {
            i += 1;
            segmentsRemaining--;
        }
        uint32_t endSegment = i;
        segments.emplace_back(startSegment + start, endSegment);
    }
    
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < segmentCount - 1; i++) {
        auto segment = segments[i];
        threads.emplace_back([segment, &job](){
            for (uint32_t i = segment.first; i < segment.second; i++) {
                job(i);
            }
        });
    }
    
    auto segment = segments.back();
    for (uint32_t i = segment.first; i < segment.second; i++) {
        job(i);
    }
    
    for (auto &thread : threads) {
        thread.join();
    }
}

std::vector<uint32_t> getClusters(Blockchain &chain, const std::unordered_map<ScriptType::Enum, uint32_t> &addressStarts, uint32_t totalAddressCount) {
    
    DisjointSets ds(totalAddressCount);
    
    auto scriptHashCount = chain.access.scripts->scriptCount<ScriptType::Enum::SCRIPTHASH>();
    
    
    segmentWork(1, scriptHashCount + 1, 8, [&ds, &addressStarts, &chain](uint32_t index) {
        ScriptPointer pointer(index, ScriptType::Enum::SCRIPTHASH);
        script::ScriptHash scripthash{*chain.access.scripts, index};
        auto wrappedAddress = scripthash.getWrappedAddress();
        if (wrappedAddress) {
            link_addresses(pointer, *wrappedAddress, ds, addressStarts);
        }
    });
    
    auto extract = [&](const Transaction &tx) {
        auto pairs = process_transaction(tx);
        for (auto &pair : pairs) {
            link_addresses(pair.first, pair.second, ds, addressStarts);
        }
        return 0;
    };
    
    chain.mapReduceTransactions(0, chain.size(), extract, [](int,int) {return 0;}, 0);
    
    segmentWork(0, totalAddressCount, 8, [&ds](uint32_t index) {
        ds.find(index);
    });
    
    std::vector<uint32_t> parents;
    parents.reserve(ds.size());
    for (uint32_t i = 0; i < totalAddressCount; i++) {
        parents.push_back(ds.find(i));
    }
    return parents;
}

uint32_t remapClusterIds(std::vector<uint32_t> &parents) {
    uint32_t placeholder = std::numeric_limits<uint32_t>::max();
    std::vector<uint32_t> newClusterIds(parents.size(), placeholder);
    uint32_t clusterCount = 0;
    for (uint32_t &clusterNum : parents) {
        uint32_t &clusterId = newClusterIds[clusterNum];
        if (clusterId == placeholder) {
            clusterId = clusterCount;
            clusterCount++;
        }
        clusterNum = clusterId;
    }
    
    std::cout << "ClusterCount is " << clusterCount << "\n";
    
    return clusterCount;
}

void recordOrderedAddresses(const std::vector<uint32_t> &parent, std::vector<uint32_t> &clusterPositions, const std::unordered_map<ScriptType::Enum, uint32_t> &addressStarts) {
    
    std::map<uint32_t, ScriptType::Enum> typeIndexes;
    for (auto &pair : addressStarts) {
        if (pair.first != ScriptType::Enum::PUBKEY) {
            typeIndexes[pair.second] = pair.first;
        }
        
    }
    
    std::vector<ScriptPointer> orderedScripts;
    orderedScripts.resize(parent.size());
    
    for (size_t i = 0; i < parent.size(); i++) {
        uint32_t &j = clusterPositions[parent[i]];
        auto it = typeIndexes.upper_bound(i);
        it--;
        uint32_t addressNum = i - it->first + 1;
        assert(addressNum != 0);
        auto addressType = it->second;
        orderedScripts[j] = ScriptPointer(addressNum, addressType);
        j++;
    }
    
    std::ofstream clusterAddressesFile("clusterAddresses.dat", std::ios::binary);
    clusterAddressesFile.write(reinterpret_cast<char *>(orderedScripts.data()), sizeof(ScriptPointer) * orderedScripts.size());
}

int main(int argc, const char * argv[]) {
    assert(argc == 2);
    
    auto progStart = std::chrono::steady_clock::now();
    
    Blockchain chain(argv[1]);
    
    std::vector<uint32_t> addressCounts;
    
    size_t totalScriptCount = chain.access.scripts->totalAddressCount();;
    
    std::unordered_map<ScriptType::Enum, uint32_t> scriptStarts;
    for (size_t i = 0; i < ScriptType::size; i++) {
        scriptStarts[ScriptType::all[i]] = 0;
        for (size_t j = 0; j < i; j++) {
            scriptStarts[ScriptType::all[i]] += chain.access.scripts->scriptCount(ScriptType::all[j]);
        }
    }
    
    auto allClusterStart = std::chrono::steady_clock::now();
    auto parent = getClusters(chain, scriptStarts, static_cast<uint32_t>(totalScriptCount));
    std::cout << "Finished main clustering in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - allClusterStart).count() / 1000000.0 << " seconds\n";
    uint32_t clusterCount = remapClusterIds(parent);
    std::cout << "Finished remapping in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - allClusterStart).count() / 1000000.0 << " seconds\n";
    std::vector<uint32_t> clusterPositions;
    clusterPositions.resize(clusterCount + 1);
    for (auto parentId : parent) {
        clusterPositions[parentId + 1]++;
    }
    
    for (size_t i = 1; i < clusterPositions.size(); i++) {
        clusterPositions[i] += clusterPositions[i-1];
    }
    
    std::cout << "Finished position tracking in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - allClusterStart).count() / 1000000.0 << " seconds\n";
    
    auto recordOrdered = std::async(std::launch::async, recordOrderedAddresses, parent, std::ref(clusterPositions), scriptStarts);
    
    segmentWork(0, ScriptType::size, ScriptType::size, [&scriptStarts, &chain, &parent](uint32_t index) {
        auto type = ScriptType::all[index];
        uint32_t startIndex = scriptStarts[type];
        uint32_t totalCount = chain.access.scripts->scriptCount(type);
        std::stringstream ss;
        ss << scriptName(type) << "_cluster_index.dat";
        std::ofstream clusterIndexFile(ss.str(), std::ios::binary);
        clusterIndexFile.write(reinterpret_cast<char *>(parent.data() + startIndex), sizeof(uint32_t) * totalCount);
    });
    
    recordOrdered.get();
    
    std::ofstream clusterOffsetFile("clusterOffsets.dat", std::ios::binary);
    clusterOffsetFile.write(reinterpret_cast<char *>(clusterPositions.data()), sizeof(uint32_t) * clusterPositions.size());
    
    std::cout << "Finished whole program in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - progStart).count() / 1000000.0 << " seconds\n";
    
    return 0;
}
