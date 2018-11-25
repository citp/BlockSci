//
//  cluster_manager.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#include <blocksci/cluster/cluster_manager.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/range_util.hpp>
#include <blocksci/core/dedup_address.hpp>
#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/scripts/scripthash_script.hpp>

#include <internal/address_info.hpp>
#include <internal/cluster_access.hpp>
#include <internal/data_access.hpp>
#include <internal/progress_bar.hpp>
#include <internal/script_access.hpp>

#include <dset/dset.h>

#include <wjfilesystem/path.h>

#include <range/v3/view/iota.hpp>
#include <fstream>
#include <future>
#include <map>

namespace {
    template <typename Job>
    void segmentWork(uint32_t start, uint32_t end, uint32_t segmentCount, Job job) {
        uint32_t total = end - start;
        
        // Don't partition over threads if there are less items than segment count
        if (total < segmentCount) {
            for (uint32_t i = start; i < end; ++i) {
                job(i);
            }
            return;
        }
        
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
}

namespace blocksci {
    ClusterManager::ClusterManager(const std::string &baseDirectory, DataAccess &access_) : access(std::make_unique<ClusterAccess>(baseDirectory, access_)), clusterCount(access->clusterCount()) {}
    
    ClusterManager::ClusterManager(ClusterManager && other) = default;
    
    ClusterManager &ClusterManager::operator=(ClusterManager && other) = default;
    
    ClusterManager::~ClusterManager() = default;
    
    Cluster ClusterManager::getCluster(const Address &address) const {
        return Cluster(access->getClusterNum(RawAddress{address.scriptNum, address.type}), *access);
    }
    
    ranges::any_view<Cluster, ranges::category::random_access | ranges::category::sized> ClusterManager::getClusters() const {
        return ranges::view::ints(0u, clusterCount)
        | ranges::view::transform([&](uint32_t clusterNum) { return Cluster(clusterNum, *access); });
    }
    
    ranges::any_view<TaggedCluster> ClusterManager::taggedClusters(const std::unordered_map<Address, std::string> &tags) const {
        return getClusters() | ranges::view::transform([tags](Cluster && cluster) -> ranges::optional<TaggedCluster> {
            return cluster.getTaggedUnsafe(tags);
        }) | flatMapOptionals();
    }
    
    struct AddressDisjointSets {
        DisjointSets disjoinSets;
        std::unordered_map<DedupAddressType::Enum, uint32_t> addressStarts;
        
        AddressDisjointSets(uint32_t totalSize, std::unordered_map<DedupAddressType::Enum, uint32_t> addressStarts_) : disjoinSets{totalSize}, addressStarts{std::move(addressStarts_)} {}
        
        uint32_t size() const {
            return disjoinSets.size();
        }
        
        void link_addresses(const Address &address1, const Address &address2) {
            auto firstAddressIndex = addressStarts.at(dedupType(address1.type)) + address1.scriptNum - 1;
            auto secondAddressIndex = addressStarts.at(dedupType(address2.type)) + address2.scriptNum - 1;
            disjoinSets.unite(firstAddressIndex, secondAddressIndex);
        }
        
        void resolveAll() {
            segmentWork(0, disjoinSets.size(), 8, [&](uint32_t index) {
                disjoinSets.find(index);
            });
        }
        
        uint32_t find(uint32_t index) {
            return disjoinSets.find(index);
        }
    };
    
    template <typename ChangeFunc>
    std::vector<std::pair<Address, Address>> processTransaction(const Transaction &tx, ChangeFunc && changeHeuristic) {
        std::vector<std::pair<Address, Address>> pairsToUnion;
        
        if (!heuristics::isCoinjoin(tx) && !tx.isCoinbase()) {
            auto inputs = tx.inputs();
            auto firstAddress = inputs[0].getAddress();
            for (uint16_t i = 1; i < inputs.size(); i++) {
                pairsToUnion.emplace_back(firstAddress, inputs[i].getAddress());
            }
            
            if (auto change = std::forward<ChangeFunc>(changeHeuristic)(tx)) {
                pairsToUnion.emplace_back(change->getAddress(), firstAddress);
            }
        }
        return pairsToUnion;
    }
    
    void linkScripthashNested(DataAccess &access, AddressDisjointSets &ds) {
        auto scriptHashCount = access.getScripts().scriptCount(DedupAddressType::SCRIPTHASH);
        
        segmentWork(1, scriptHashCount + 1, 8, [&ds, &access](uint32_t index) {
            Address pointer(index, AddressType::SCRIPTHASH, access);
            script::ScriptHash scripthash{index, access};
            auto wrappedAddress = scripthash.getWrappedAddress();
            if (wrappedAddress) {
                ds.link_addresses(pointer, *wrappedAddress);
            }
        });
    }
    
    template <typename ChangeFunc>
    std::vector<uint32_t> createClusters(BlockRange &chain, std::unordered_map<DedupAddressType::Enum, uint32_t> addressStarts, uint32_t totalScriptCount, ChangeFunc && changeHeuristic) {
        
        AddressDisjointSets ds(totalScriptCount, std::move(addressStarts));
        
        auto &access = chain.getAccess();
        
        linkScripthashNested(access, ds);
        
        auto extract = [&](const BlockRange &blocks, int threadNum) {
            auto progressThread = static_cast<int>(std::thread::hardware_concurrency()) - 1;
            auto progressBar = makeProgressBar(blocks.endTxIndex() - blocks.firstTxIndex(), [=]() {});
            if (threadNum != progressThread) {
                progressBar.setSilent();
            }
            uint32_t txNum = 0;
            for (auto block : blocks) {
                for (auto tx : block) {
                    auto pairs = processTransaction(tx, changeHeuristic);
                    for (auto &pair : pairs) {
                        ds.link_addresses(pair.first, pair.second);
                    }
                    progressBar.update(txNum);
                    txNum++;
                }
            }
            return 0;
        };
        
        chain.mapReduce<int>(extract, [](int &a,int &) -> int & {return a;});
        
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
        
        return clusterCount;
    }
    
    void recordOrderedAddresses(const std::vector<uint32_t> &parent, std::vector<uint32_t> &clusterPositions, const std::unordered_map<DedupAddressType::Enum, uint32_t> &scriptStarts, std::ofstream &clusterAddressesFile) {
        
        std::map<uint32_t, DedupAddressType::Enum> typeIndexes;
        for (auto &pair : scriptStarts) {
            typeIndexes[pair.second] = pair.first;
        }
        
        std::vector<DedupAddress> orderedScripts;
        orderedScripts.resize(parent.size());
        
        for (uint32_t i = 0; i < parent.size(); i++) {
            uint32_t &j = clusterPositions[parent[i]];
            auto it = typeIndexes.upper_bound(i);
            it--;
            uint32_t addressNum = i - it->first + 1;
            auto addressType = it->second;
            orderedScripts[j] = DedupAddress(addressNum, addressType);
            j++;
        }
        
        clusterAddressesFile.write(reinterpret_cast<char *>(orderedScripts.data()), static_cast<long>(sizeof(DedupAddress) * orderedScripts.size()));
    }
    
    void prepareClusterDataLocation(const std::string &outputPath, bool overwrite) {
        auto outputLocation = filesystem::path{outputPath};
        
        std::string offsetFile = ClusterAccess::offsetFilePath(outputPath);
        std::string addressesFile = ClusterAccess::addressesFilePath(outputPath);
        std::vector<std::string> clusterIndexPaths;
        clusterIndexPaths.resize(DedupAddressType::size);
        for (auto dedupType : DedupAddressType::allArray()) {
            clusterIndexPaths[static_cast<size_t>(dedupType)] = ClusterAccess::typeIndexFilePath(outputPath, dedupType);
        }
        
        std::vector<std::string> allPaths = clusterIndexPaths;
        allPaths.push_back(offsetFile);
        allPaths.push_back(addressesFile);
        
        // Prepare cluster folder or fail
        auto outputLocationPath = filesystem::path{outputLocation};
        if (outputLocationPath.exists()) {
            if (!outputLocationPath.is_directory()) {
                throw std::runtime_error{"Path must be to a directory, not a file"};
            }
            if (!overwrite) {
                for (auto &path : allPaths) {
                    auto filePath = filesystem::path{path};
                    if (filePath.exists()) {
                        std::stringstream ss;
                        ss << "Overwrite is off, but " << filePath << " exists already";
                        throw std::runtime_error{ss.str()};
                    }
                }
            } else {
                for (auto &path : allPaths) {
                    auto filePath = filesystem::path{path};
                    if (filePath.exists()) {
                        filePath.remove_file();
                    }
                }
            }
        }
    }
    
    void serializeClusterData(const ScriptAccess &scripts, const std::string &outputPath, const std::vector<uint32_t> &parent, const std::unordered_map<DedupAddressType::Enum, uint32_t> &scriptStarts, uint32_t clusterCount) {
        auto outputLocation = filesystem::path{outputPath};
        std::string offsetFile = ClusterAccess::offsetFilePath(outputPath);
        std::string addressesFile = ClusterAccess::addressesFilePath(outputPath);
        std::vector<std::string> clusterIndexPaths;
        clusterIndexPaths.resize(DedupAddressType::size);
        for (auto dedupType : DedupAddressType::allArray()) {
            clusterIndexPaths[static_cast<size_t>(dedupType)] = ClusterAccess::typeIndexFilePath(outputPath, dedupType);
        }
        
        filesystem::create_directory(outputLocation);
        // Generate cluster files
        
        std::vector<uint32_t> clusterPositions;
        clusterPositions.resize(clusterCount + 1);
        for (auto parentId : parent) {
            clusterPositions[parentId + 1]++;
        }
        
        for (size_t i = 1; i < clusterPositions.size(); i++) {
            clusterPositions[i] += clusterPositions[i-1];
        }
        std::ofstream clusterAddressesFile(addressesFile, std::ios::binary);
        auto recordOrdered = std::async(std::launch::async, recordOrderedAddresses, parent, std::ref(clusterPositions), scriptStarts, std::ref(clusterAddressesFile));
        
        segmentWork(0, DedupAddressType::size, DedupAddressType::size, [&](uint32_t index) {
            auto type = static_cast<DedupAddressType::Enum>(index);
            uint32_t startIndex = scriptStarts.at(type);
            uint32_t totalCount = scripts.scriptCount(type);
            std::ofstream file{clusterIndexPaths[index], std::ios::binary};
            file.write(reinterpret_cast<const char *>(parent.data() + startIndex), sizeof(uint32_t) * totalCount);
        });
        
        recordOrdered.get();
        
        std::ofstream clusterOffsetFile(offsetFile, std::ios::binary);
        clusterOffsetFile.write(reinterpret_cast<char *>(clusterPositions.data()), static_cast<long>(sizeof(uint32_t) * clusterPositions.size()));
    }
    
    template <typename ChangeFunc>
    ClusterManager createClusteringImpl(BlockRange &chain, ChangeFunc && changeHeuristic, const std::string &outputPath, bool overwrite) {
        prepareClusterDataLocation(outputPath, overwrite);
        
        // Perform clustering
        
        auto &scripts = chain.getAccess().getScripts();
        size_t totalScriptCount = scripts.totalAddressCount();
        
        std::unordered_map<DedupAddressType::Enum, uint32_t> scriptStarts;
        {
            std::vector<uint32_t> starts(DedupAddressType::size);
            for (size_t i = 0; i < DedupAddressType::size; i++) {
                if (i > 0) {
                    starts[i] = scripts.scriptCount(static_cast<DedupAddressType::Enum>(i - 1)) + starts[i - 1];
                }
                scriptStarts[static_cast<DedupAddressType::Enum>(i)] = starts[i];
            }
        }
        
        auto parent = createClusters(chain, scriptStarts, static_cast<uint32_t>(totalScriptCount), std::forward<ChangeFunc>(changeHeuristic));
        uint32_t clusterCount = remapClusterIds(parent);
        serializeClusterData(scripts, outputPath, parent, scriptStarts, clusterCount);
        return {filesystem::path{outputPath}.str(), chain.getAccess()};
    }
    
    ClusterManager ClusterManager::createClustering(BlockRange &chain, const heuristics::ChangeHeuristic &changeHeuristic, const std::string &outputPath, bool overwrite) {
        
        auto changeHeuristicL = [&changeHeuristic](const Transaction &tx) -> ranges::optional<Output> {
            return changeHeuristic.uniqueChange(tx);
        };
        
        return createClusteringImpl(chain, changeHeuristicL, outputPath, overwrite);
    }
    
    ClusterManager ClusterManager::createClustering(BlockRange &chain, const std::function<ranges::optional<Output>(const Transaction &tx)> &changeHeuristic, const std::string &outputPath, bool overwrite) {
        return createClusteringImpl(chain, changeHeuristic, outputPath, overwrite);
    }
} // namespace blocksci


