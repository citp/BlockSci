//
//  main.cpp
//  blocksci-test
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <dset/dset.h>

#include <blocksci/blocksci.hpp>
#include <blocksci/data_access.hpp>

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <future>

#include <fstream>

using namespace blocksci;


bool isCoinjoinFast(const Transaction &tx, const std::vector<const Output *> &spentOutputs) {
    if (tx.inputCount() < 2 || tx.outputCount() < 3) {
        return false;
    }
    
    uint16_t participantCount = (tx.outputCount() + 1) / 2;
    if (participantCount > tx.inputCount()) {
        return false;
    }
    
    std::unordered_set<AddressPointer> inputAddresses;
    for (auto spentOutput : spentOutputs) {
        inputAddresses.insert(spentOutput->getAddressPointer());
    }
    
    if (participantCount > inputAddresses.size()) {
        return false;
    }
    
    std::unordered_map<uint64_t, uint16_t> outputValues;
    for (auto &output : tx.outputs()) {
        outputValues[output.getValue()]++;
    }
    
    using pair_type = decltype(outputValues)::value_type;
    auto pr = std::max_element(std::begin(outputValues), std::end(outputValues),
                               [] (const pair_type & p1, const pair_type & p2) {
                                   return p1.second < p2.second;
                               }
                               );
    
    if (pr->second != participantCount) {
        return false;
    }
    
    if (pr->first == 546 || pr->first == 2730) {
        return false;
    }
    
    return true;
}

const Output * getChange(const Transaction &tx, uint64_t smallestInput) {
    if (tx.outputCount() <= 1) {
        return nullptr;
    }
    
    uint16_t spendableCount = 0;
    const Output *change = nullptr;
    for (const auto &output : tx.outputs()) {
        auto addressPointer = output.getAddressPointer();
        if (addressPointer.isSpendable()) {
            if (output.getValue() < smallestInput && addressPointer.getFirstTransactionIndex() == tx.txNum) {
                if (change) {
                    // Multiple possible change addresses
                    return nullptr;
                }
                change = &output;
            }
            spendableCount++;
        }
    }
    
    if (spendableCount < 2) {
        return nullptr;
    }
    
    return change;
}

std::vector<std::pair<AddressPointer, AddressPointer>> process_transaction(const Transaction &tx) {
    std::vector<std::pair<AddressPointer, AddressPointer>> pairsToUnion;
    
    if (!isCoinjoin(tx) && !isCoinbase(tx)) {
        auto inputs = tx.inputs();
        uint64_t smallestInput = inputs.front().getValue();
        auto firstAddress = inputs.front().getAddressPointer();
        if (tx.inputCount() > 1) {
            for (uint16_t i = 1; i < tx.inputCount(); i++) {
                smallestInput = std::min(smallestInput, inputs[i].getValue());
                pairsToUnion.emplace_back(firstAddress, inputs[i].getAddressPointer());
            }
        }
        
        auto change = getChange(tx, smallestInput);
        
        if (change) {
            pairsToUnion.emplace_back(change->getAddressPointer(), firstAddress);
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

std::vector<uint32_t> getClusters(Blockchain &chain, std::unordered_map<AddressPointer, uint32_t> &addressesMapped, uint32_t totalAddressCount) {
    
    DisjointSets ds(totalAddressCount);
    
    auto scriptHashCount = chain.access.scripts.addressCount<ScriptType::Enum::SCRIPTHASH>();
    
    
    segmentWork(1, scriptHashCount + 1, 8, [&ds, &addressesMapped](uint32_t index) {
        AddressPointer pointer(index, ScriptType::Enum::SCRIPTHASH);
        auto address = pointer.getAddress();
        auto addressPointer = dynamic_cast<address::ScriptHash *>(address.get());
        if (addressPointer && addressPointer->wrappedAddressPointer.addressNum > 0) {
            auto wrappedPointer = addressPointer->wrappedAddressPointer;
            ds.unite(addressesMapped[pointer], addressesMapped[wrappedPointer]);
        }
    });
    
    auto extract = [&](const Transaction &tx) {
        auto pairs = process_transaction(tx);
        for (auto &pair : pairs) {
            ds.unite(addressesMapped[pair.first], addressesMapped[pair.second]);
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

void recordOrderedAddresses(const std::vector<uint32_t> &parent, std::vector<uint32_t> &clusterPositions, const std::vector<AddressPointer> &idensMapped) {
    
    std::vector<AddressPointer> orderedAddresses;
    orderedAddresses.resize(parent.size());
    
    for (size_t i = 0; i < parent.size(); i++) {
        uint32_t &j = clusterPositions[parent[i]];
        orderedAddresses[j] = idensMapped[i];
        j++;
    }
    
    std::ofstream clusterAddressesFile("clusterAddresses.dat", std::ios::binary);
    clusterAddressesFile.write(reinterpret_cast<char *>(orderedAddresses.data()), sizeof(AddressPointer) * orderedAddresses.size());
}

int main(int argc, const char * argv[]) {
    assert(argc == 2);
    
    auto progStart = std::chrono::steady_clock::now();
    
    Blockchain chain(argv[1]);
    
    
    
    std::vector<uint32_t> addressCounts;
    
    constexpr std::array<ScriptType::Enum, 5> types = {{
        ScriptType::Enum::NONSTANDARD,
        ScriptType::Enum::PUBKEYHASH,
        ScriptType::Enum::SCRIPTHASH,
        ScriptType::Enum::MULTISIG,
        ScriptType::Enum::NULL_DATA
    }};
    
    uint32_t totalAddressCount = 0;
    for (auto type : types) {
        totalAddressCount += chain.access.scripts.addressCount(type);
    }
    
    std::unordered_map<AddressPointer, uint32_t> addressesMapped;
    std::vector<AddressPointer> idensMapped;
    idensMapped.reserve(totalAddressCount);
    addressesMapped.reserve(totalAddressCount);
    
    std::unordered_map<ScriptType::Enum, uint32_t> addressStarts;
    
    uint32_t index = 0;
    for (size_t i = 0; i < types.size(); i++) {
        auto type = types[i];
        addressStarts[type] = index;
        auto count = chain.access.scripts.addressCount(type);
        for (uint32_t i = 1; i <= count; i++) {
            auto pointer = AddressPointer{i, type};
            addressesMapped[pointer] = index;
            idensMapped.push_back(pointer);
            index++;
        }
    }
    
    
    auto allClusterStart = std::chrono::steady_clock::now();
    auto parent = getClusters(chain, addressesMapped, totalAddressCount);
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
    
    
    auto recordOrdered = std::async(std::launch::async, recordOrderedAddresses, parent, std::ref(clusterPositions), idensMapped);
    
    segmentWork(0, types.size(), types.size(), [&types, &addressStarts, &chain, &parent](uint32_t index) {
        auto type = types[index];
        uint32_t startIndex = addressStarts[type];
        uint32_t totalCount = chain.access.scripts.addressCount(type);
        std::ofstream clusterIndexFile(GetTxnOutputType(type) + "_cluster_index.dat", std::ios::binary);
        clusterIndexFile.write(reinterpret_cast<char *>(parent.data() + startIndex), sizeof(uint32_t) * totalCount);
    });
    
    recordOrdered.get();
    
    std::ofstream clusterOffsetFile("clusterOffsets.dat", std::ios::binary);
    clusterOffsetFile.write(reinterpret_cast<char *>(clusterPositions.data()), sizeof(uint32_t) * clusterPositions.size());
    
    std::cout << "Finished whole program in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - progStart).count() / 1000000.0 << " seconds\n";

    return 0;
}
