//
//  cluster_manager.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#include "cluster_manager.hpp"
#include "cluster.hpp"

#include <blocksci/address/address_info.hpp>
#include <blocksci/address/dedup_address.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/util/progress_bar.hpp>

#include <dset/dset.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

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
    ClusterManager::ClusterManager(const boost::filesystem::path &baseDirectory, blocksci::DataAccess &access_) :
    clusterOffsetFile(baseDirectory/"clusterOffsets"),
    clusterScriptsFile(baseDirectory/"clusterAddresses"),
    scriptClusterIndexFiles(blocksci::apply(blocksci::DedupAddressType::all(), [&] (auto tag) {
        std::stringstream ss;
        ss << blocksci::dedupAddressName(tag) << "_cluster_index";
        return baseDirectory/ss.str();
    })),
    access(access_)  {}
    
    uint32_t ClusterManager::clusterCount() const {
        return static_cast<uint32_t>(clusterOffsetFile.size());
    }
    
    template<blocksci::DedupAddressType::Enum type>
    struct ClusterNumFunctor {
        static uint32_t f(const ClusterManager *cm, uint32_t scriptNum) {
            return cm->getClusterNumImpl<type>(scriptNum);
        }
    };
    
    
    uint32_t ClusterManager::getClusterNum(const blocksci::Address &address) const {
        static auto table = blocksci::make_dynamic_table<blocksci::DedupAddressType, ClusterNumFunctor>();
        static constexpr std::size_t size = DedupAddressType::size;
        
        auto index = static_cast<size_t>(dedupType(address.type));
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](this, address.scriptNum);
    }
    
    Cluster ClusterManager::getCluster(const blocksci::Address &address) const {
        return Cluster(getClusterNum(address), *this);
    }
    
    std::vector<TaggedCluster> ClusterManager::taggedClusters(const std::unordered_map<blocksci::Address, std::string> &tags) {
        std::vector<TaggedCluster> taggedClusters;
        for (auto cluster : getClusters()) {
            auto taggedAddresses = cluster.taggedAddresses(tags);
            if (!taggedAddresses.empty()) {
                taggedClusters.emplace_back(cluster, std::move(taggedAddresses));
            }
        }
        return taggedClusters;
    }
    
    uint32_t ClusterManager::getClusterSize(uint32_t clusterNum) const {
        auto clusterOffset = *clusterOffsetFile.getData(clusterNum);
        auto clusterSize = clusterOffset;
        if (clusterNum > 0) {
            clusterSize -= *clusterOffsetFile.getData(clusterNum - 1);
        }
        return clusterSize;
    }
    
    std::vector<uint32_t> ClusterManager::getClusterSizes() const {
        auto tot = clusterCount();
        std::vector<uint32_t> clusterSizes;
        clusterSizes.resize(tot);
        
        clusterSizes[tot - 1] = *clusterOffsetFile.getData(tot - 1);
        for (uint32_t i = 2; i <= tot; i++) {
            clusterSizes[tot - i] = *clusterOffsetFile.getData(tot - i);
            clusterSizes[(tot - i) + 1] -= clusterSizes[tot - i];
        }
        return clusterSizes;
    }
    
    ranges::iterator_range<const blocksci::DedupAddress *> ClusterManager::getClusterScripts(uint32_t clusterNum) const {
        auto nextClusterOffset = *clusterOffsetFile.getData(clusterNum);
        uint32_t clusterOffset = 0;
        if (clusterNum > 0) {
            clusterOffset = *clusterOffsetFile.getData(clusterNum - 1);
        }
        auto clusterSize = nextClusterOffset - clusterOffset;
        
        auto firstAddressOffset = clusterScriptsFile.getData(clusterOffset);
        
        return ranges::make_iterator_range(firstAddressOffset, firstAddressOffset + clusterSize);
    }
    
    std::vector<std::pair<Address, Address>> processTransaction(const Transaction &tx, const heuristics::ChangeHeuristic &changeHeuristic) {
        std::vector<std::pair<Address, Address>> pairsToUnion;
        
        if (!heuristics::isCoinjoin(tx) && !tx.isCoinbase()) {
            auto inputs = tx.inputs();
            auto firstAddress = inputs[0].getAddress();
            for (uint16_t i = 1; i < inputs.size(); i++) {
                pairsToUnion.emplace_back(firstAddress, inputs[i].getAddress());
            }
            
            if (auto change = changeHeuristic.uniqueChange(tx)) {
                pairsToUnion.emplace_back(change->getAddress(), firstAddress);
            }
        }
        return pairsToUnion;
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
    
    std::vector<uint32_t> createClusters(Blockchain &chain, std::unordered_map<DedupAddressType::Enum, uint32_t> addressStarts, uint32_t totalScriptCount, const heuristics::ChangeHeuristic &changeHeuristic) {
        
        AddressDisjointSets ds(totalScriptCount, std::move(addressStarts));
        
        auto &access = chain.getAccess();
        
        auto scriptHashCount = chain.addressCount(AddressType::SCRIPTHASH);
        
        
        segmentWork(1, scriptHashCount + 1, 8, [&ds, &access](uint32_t index) {
            Address pointer(index, AddressType::SCRIPTHASH, access);
            script::ScriptHash scripthash{index, access};
            auto wrappedAddress = scripthash.getWrappedAddress();
            if (wrappedAddress) {
                ds.link_addresses(pointer, *wrappedAddress);
            }
        });
        
        
        auto extract = [&](const std::vector<Block> &blocks, int threadNum) {
            uint32_t totalTxCount = 0;
            auto progressThread = static_cast<int>(std::thread::hardware_concurrency()) - 1;
            if (threadNum == progressThread) {
                for (auto &block : blocks) {
                    totalTxCount += block.size();
                }
            }
            auto progressBar = makeProgressBar(totalTxCount, [=]() {});
            if (threadNum != progressThread) {
                progressBar.setSilent();
            }
            uint32_t txNum = 0;
            for (auto &block : blocks) {
                RANGES_FOR(auto tx, block) {
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
        
        chain.mapReduce<int>(0, chain.size(), extract, [](int &a,int &) -> int & {return a;});
        
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
    
    void recordOrderedAddresses(const std::vector<uint32_t> &parent, std::vector<uint32_t> &clusterPositions, const std::unordered_map<DedupAddressType::Enum, uint32_t> &scriptStarts, boost::filesystem::ofstream &clusterAddressesFile) {
        
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
        
        clusterAddressesFile.write(reinterpret_cast<char *>(orderedScripts.data()), sizeof(DedupAddress) * orderedScripts.size());
    }
    
    
    ClusterManager ClusterManager::createClustering(Blockchain &chain, const heuristics::ChangeHeuristic &changeHeuristic, const std::string &outputPath, bool overwrite) {
        
        auto outputLocation = boost::filesystem::path{outputPath};
        boost::filesystem::path offsetFile = outputLocation/"clusterOffsets.dat";
        boost::filesystem::path addressesFile = outputLocation/"clusterAddresses.dat";
        std::vector<boost::filesystem::path> clusterIndexPaths;
        for_each(DedupAddressType::all(), [&](auto dedupType) {
            std::stringstream ss;
            ss << dedupAddressName(dedupType) << "_cluster_index.dat";
            clusterIndexPaths.push_back(outputLocation/ss.str());
        });
        
        std::vector<boost::filesystem::path> allPaths = clusterIndexPaths;
        allPaths.push_back(offsetFile);
        allPaths.push_back(addressesFile);
        
        // Prepare cluster folder or fail
        if (boost::filesystem::exists(outputLocation)) {
            if (!boost::filesystem::is_directory(outputLocation)) {
                throw std::runtime_error{"Path must be to a directory, not a file"};
            }
            if (!overwrite) {
                for (const auto &path : allPaths) {
                    if (boost::filesystem::exists(path)) {
                        std::stringstream ss;
                        ss << "Overwrite is off, but " << path << " exists already";
                        throw std::runtime_error{ss.str()};
                    }
                }
            } else {
                for (const auto &path : allPaths) {
                    if (boost::filesystem::exists(path)) {
                        boost::filesystem::remove(path);
                    }
                }
            }
        }
        boost::filesystem::create_directories(outputLocation);
        
        // Generate cluster files
        boost::filesystem::ofstream clusterAddressesFile(addressesFile, std::ios::binary);
        boost::filesystem::ofstream clusterOffsetFile(offsetFile, std::ios::binary);
        // Perform clustering
        
        auto &scripts = chain.getAccess().scripts;
        size_t totalScriptCount = scripts.totalAddressCount();
        
        std::unordered_map<DedupAddressType::Enum, uint32_t> scriptStarts;
        for_each(DedupAddressType::all(), [&](auto type) {
            scriptStarts[type] = 0;
            for (size_t j = 0; j < static_cast<size_t>(type); j++) {
                scriptStarts[type] += scripts.scriptCount(static_cast<DedupAddressType::Enum>(j));
            }
        });
        
        auto parent = createClusters(chain, scriptStarts, static_cast<uint32_t>(totalScriptCount), changeHeuristic);
        uint32_t clusterCount = remapClusterIds(parent);
        std::vector<uint32_t> clusterPositions;
        clusterPositions.resize(clusterCount + 1);
        for (auto parentId : parent) {
            clusterPositions[parentId + 1]++;
        }
        
        for (size_t i = 1; i < clusterPositions.size(); i++) {
            clusterPositions[i] += clusterPositions[i-1];
        }
        
        auto recordOrdered = std::async(std::launch::async, recordOrderedAddresses, parent, std::ref(clusterPositions), scriptStarts, std::ref(clusterAddressesFile));
        
        segmentWork(0, DedupAddressType::size, DedupAddressType::size, [&](uint32_t index) {
            auto type = static_cast<DedupAddressType::Enum>(index);
            uint32_t startIndex = scriptStarts[type];
            uint32_t totalCount = scripts.scriptCount(type);
            boost::filesystem::ofstream file = {clusterIndexPaths[index], std::ios::binary};
            file.write(reinterpret_cast<char *>(parent.data() + startIndex), sizeof(uint32_t) * totalCount);
        });
        
        recordOrdered.get();
        
        clusterOffsetFile.write(reinterpret_cast<char *>(clusterPositions.data()), sizeof(uint32_t) * clusterPositions.size());
        return ClusterManager{outputLocation, chain.getAccess()};
    }
} // namespace blocksci


