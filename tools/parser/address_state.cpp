//
//  address_state.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "address_state.hpp"
#include "parser_configuration.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/core/address_info.hpp>
#include <blocksci/util/hash.hpp>

#include <boost/filesystem/fstream.hpp>

#include <string>
#include <sstream>

namespace {
    static constexpr auto multiAddressFileName = "multi";
    static constexpr auto bloomFileName = "bloom_";
    static constexpr auto scriptCountsFileName = "scriptCounts.txt";
}

AddressState::AddressState(boost::filesystem::path path_, HashIndexCreator &hashDb) : path(std::move(path_)), db(hashDb), addressBloomFilters(blocksci::apply(blocksci::DedupAddressType::all(), [&] (auto tag) {
    return std::make_unique<AddressBloomFilter<tag>>(path/std::string(bloomFileName));
}))  {
    blocksci::for_each(multiAddressMaps, [&](auto &multiAddressMap) {
        std::stringstream ss;
        ss << multiAddressFileName << "_" << dedupAddressName(multiAddressMap.type) << ".dat";
        multiAddressMap.unserialize((path/ss.str()).native());
    });
    
    boost::filesystem::ifstream inputFile(path/std::string(scriptCountsFileName));
    
    if (inputFile) {
        uint32_t value;
        while ( inputFile >> value ) {
            scriptIndexes.push_back(value);
        }
    } else {
        for (size_t i = 0; i < blocksci::DedupAddressType::size; i++) {
            scriptIndexes.push_back(1);
        }
    }
}

AddressState::~AddressState() {
    blocksci::for_each(multiAddressMaps, [&](auto &multiAddressMap) {
        std::stringstream ss;
        ss << multiAddressFileName << "_" << dedupAddressName(multiAddressMap.type) << ".dat";
        multiAddressMap.serialize((path/ss.str()).native());
    });
    
    boost::filesystem::ofstream outputFile(path/std::string(scriptCountsFileName));
    for (auto value : scriptIndexes) {
        outputFile << value << " ";
    }
}

uint32_t AddressState::getNewAddressIndex(blocksci::DedupAddressType::Enum type) {
    auto &count = scriptIndexes[static_cast<uint8_t>(type)];
    auto scriptNum = count;
    count++;
    return scriptNum;
}

void AddressState::rollback(const blocksci::State &state) {
    blocksci::for_each(multiAddressMaps, [&](auto &multiAddressMap) {
        for (auto multiAddressIt = multiAddressMap.begin(); multiAddressIt != multiAddressMap.end(); ++multiAddressIt) {
            auto count = state.scriptCounts[static_cast<size_t>(multiAddressMap.type)];
            if (multiAddressIt->second >= count) {
                multiAddressMap.erase(multiAddressIt);
            }
        }
    });
}

void AddressState::reset(const blocksci::State &state) {
    reloadBloomFilters();
    scriptIndexes.clear();
    for (auto size : state.scriptCounts) {
        scriptIndexes.push_back(size);
    }
}
