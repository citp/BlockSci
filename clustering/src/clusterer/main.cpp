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

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <future>

#include <fstream>

using namespace blocksci;

std::vector<std::pair<Script, Script>> process_transaction(const Transaction &tx) {
    std::vector<std::pair<Script, Script>> pairsToUnion;
    
    if (!isCoinjoin(tx) && !isCoinbase(tx)) {
        auto inputs = tx.inputs();
        auto firstAddress = inputs[0].getAddress();
        for (uint16_t i = 1; i < inputs.size(); i++) {
            pairsToUnion.emplace_back(firstAddress, inputs[i].getAddress());
        }
        
        if (auto change = getChangeOutput(tx)) {
            pairsToUnion.emplace_back(change->getAddress(), firstAddress);
        }
    }
    return pairsToUnion;
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

struct AddressDisjointSets {
    DisjointSets ds;
    std::unordered_map<ScriptType::Enum, uint32_t> addressStarts;

    AddressDisjointSets(uint32_t totalSize, std::unordered_map<ScriptType::Enum, uint32_t> addressStarts_), ds(totalSize), addressStarts(std::move(addressStarts)) {}

    uint32_t size() const {
        return ds.size();
    }

    void link_addresses(const Script &address1, const Script &address2, DisjointSets &sets, const std::unordered_map<ScriptType::Enum, uint32_t> &addressStarts) {
        auto firstAddressIndex = addressStarts.at(address1.type) + address1.scriptNum - 1;
        auto secondAddressIndex = addressStarts.at(address2.type) + address2.scriptNum - 1;
        ds.unite(firstAddressIndex, secondAddressIndex);
    }

    void resolveAll() {
        segmentWork(0, ds.size(), 8, [&ds](uint32_t index) {
            ds.find(index);
        });
    }

    uint32_t find(uint32_t index) {
        return ds.find(index);
    }
};

std::vector<uint32_t> getClusters(Blockchain &chain, std::unordered_map<ScriptType::Enum, uint32_t> addressStarts, uint32_t totalScriptCount) {
    
    AddressDisjointSets ds(totalScriptCount, std::move(addressStarts));
    
    auto &scripts = *chain.access->scripts;

    auto scriptHashCount = scripts.scriptCount<ScriptType::Enum::SCRIPTHASH>();
    
    
    segmentWork(1, scriptHashCount + 1, 8, [&ds, &scripts](uint32_t index) {
        Script pointer(index, ScriptType::Enum::SCRIPTHASH);
        script::ScriptHash scripthash{scripts, index};
        auto wrappedAddress = scripthash.getWrappedAddress();
        if (wrappedAddress) {
            ds.link_addresses(pointer, *wrappedAddress);
        }
    });
    
    auto extract = [&](const Transaction &tx) {
        auto pairs = process_transaction(tx);
        for (auto &pair : pairs) {
            ds.link_addresses(pair.first, pair.second);
        }
        return 0;
    };
    
    chain.mapReduce<int, int>(0, chain.size() - 10, extract, [](int &a,int &) -> int & {return a;}, 0);
    
    ds.resolveAll();
    
    std::vector<uint32_t> parents;
    parents.reserve(ds.size());
    for (uint32_t i = 0; i < totalScriptCount; i++) {
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

void recordOrderedAddresses(const std::vector<uint32_t> &parent, std::vector<uint32_t> &clusterPositions, const std::unordered_map<ScriptType::Enum, uint32_t> &scriptStarts) {
    
    std::map<uint32_t, ScriptType::Enum> typeIndexes;
    for (auto &pair : scriptStarts) {
        typeIndexes[pair.second] = pair.first;
    }
    
    std::vector<Script> orderedScripts;
    orderedScripts.resize(parent.size());
    
    for (uint32_t i = 0; i < parent.size(); i++) {
        uint32_t &j = clusterPositions[parent[i]];
        auto it = typeIndexes.upper_bound(i);
        it--;
        uint32_t addressNum = i - it->first + 1;
        auto addressType = it->second;
        orderedScripts[j] = Script(addressNum, addressType);
        j++;
    }
    
    std::ofstream clusterAddressesFile("clusterAddresses.dat", std::ios::binary);
    clusterAddressesFile.write(reinterpret_cast<char *>(orderedScripts.data()), sizeof(Script) * orderedScripts.size());
}

int main(int argc, const char * argv[]) {
    assert(argc == 2);
    
    auto progStart = std::chrono::steady_clock::now();
    
    Blockchain chain(argv[1]);
    
    auto &scripts = *chain.access->scripts;
    size_t totalScriptCount = scripts.totalAddressCount();;
    
    std::unordered_map<ScriptType::Enum, uint32_t> scriptStarts;
    for (size_t i = 0; i < ScriptType::size; i++) {
        scriptStarts[ScriptType::all[i]] = 0;
        for (size_t j = 0; j < i; j++) {
            scriptStarts[ScriptType::all[i]] += scripts.scriptCount(ScriptType::all[j]);
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
    
    segmentWork(0, ScriptType::size, ScriptType::size, [&scriptStarts, &scripts, &parent](uint32_t index) {
        auto type = ScriptType::all[index];
        uint32_t startIndex = scriptStarts[type];
        uint32_t totalCount = scripts.scriptCount(type);
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
