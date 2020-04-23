//
//  main.cpp
//
//  blocksci_check_integrity
//  Created by Malte Möser on 3/6/20.
//

#include <blocksci/address.hpp>
#include <blocksci/core/dedup_address.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/script.hpp>


#include <internal/address_index.hpp>
#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>
#include <internal/dedup_address_info.hpp>
#include <internal/hash_index.hpp>
#include <internal/script_access.hpp>

#include <range/v3/utility/optional.hpp>

#include <openssl/sha.h>

#include <clipp.h>

#include <iostream>
#include <fstream>

using namespace blocksci;

/**
 Compute a checksum over block data.
 */
uint256 compute_block_hash(const ChainAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    for(int i = 0; i < access.blockCount(); i++) {
        const RawBlock *block = access.getBlock(i);
        SHA256_Update(&sha256, &block->baseSize, 4);
        SHA256_Update(&sha256, &block->bits, 4);
        SHA256_Update(&sha256, &block->coinbaseOffset, 8);
        SHA256_Update(&sha256, &block->firstTxIndex, 4);
        SHA256_Update(&sha256, &block->hash, 32);
        SHA256_Update(&sha256, &block->height, 4);
        SHA256_Update(&sha256, &block->inputCount, 4);
        SHA256_Update(&sha256, &block->nonce, 4);
        SHA256_Update(&sha256, &block->outputCount, 4);
        SHA256_Update(&sha256, &block->realSize, 4);
        SHA256_Update(&sha256, &block->timestamp, 4);
        SHA256_Update(&sha256, &block->txCount, 4);
        SHA256_Update(&sha256, &block->version, 4);
    }
    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
 Compute a checksum over core transaction data.
 */
uint256 compute_txdata_hash(const ChainAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    for(size_t i = 0; i < access.txCount(); i++) {
        // no additional padding in RawTransaction, can simply hash struct
        const RawTransaction *tx = access.getTx(i);
        SHA256_Update(&sha256, tx, tx->serializedSize());
    }
    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
 Compute a checksum over additional data (kept in separate files that are memory-mapped on-demand).
 - Input sequence numbers
 - Indexes of outputs spent by inputs
 - Transaction version
 - Transaction hashes
 - Index of first input for each transaction
 - Index of first output for each transaction
 */
uint256 compute_additional_data_hash(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const ChainAccess &chainAccess = access.getChain();
    auto chainDirectory = access.config.chainDirectory();

    FixedSizeFileMapper<uint32_t> sequenceFile(chainAccess.sequenceFilePath(chainDirectory));
    for(uint64_t i = 0; i < chainAccess.inputCount(); ++i) {
        SHA256_Update(&sha256, sequenceFile[i], 4);
    }

    FixedSizeFileMapper<uint16_t> spentOutNumFile(chainAccess.inputSpentOutNumFilePath(chainDirectory));
    for(uint64_t i = 0; i < chainAccess.inputCount(); ++i) {
        SHA256_Update(&sha256, spentOutNumFile[i], 2);
    }

    FixedSizeFileMapper<int32_t> txVersionFile(chainAccess.txVersionFilePath(chainDirectory));
    for(uint32_t i = 0; i < chainAccess.txCount(); ++i) {
        SHA256_Update(&sha256, txVersionFile[i], 4);
    }

    FixedSizeFileMapper<uint256> txHashesFile(chainAccess.txHashesFilePath(chainDirectory));
    for(uint32_t i = 0; i < chainAccess.txCount(); ++i) {
        SHA256_Update(&sha256, txHashesFile[i], 32);
    }

    FixedSizeFileMapper<uint64_t> txFirstInputFile(chainAccess.firstInputFilePath(chainDirectory));
    for(uint32_t i = 0; i < chainAccess.txCount(); ++i) {
        SHA256_Update(&sha256, txFirstInputFile[i], 8);
    }

    FixedSizeFileMapper<uint64_t> txFirstOutputFile(chainAccess.firstOutputFilePath(chainDirectory));
    for(uint32_t i = 0; i < chainAccess.txCount(); ++i) {
        SHA256_Update(&sha256, txFirstOutputFile[i], 8);
    }

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

template<DedupAddressType::Enum dedupType>
uint256 compute_scriptdata_hash(const DataAccess &access);

/**
 Compute a checksum over pubkey data.
 */
template<>
uint256 compute_scriptdata_hash<DedupAddressType::Enum::PUBKEY>(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    auto scripts = &access.getScripts();
    constexpr DedupAddressType::Enum dedupType = DedupAddressType::PUBKEY;
    auto scriptCount = scripts->scriptCount(dedupType);

    for(uint32_t i = 1; i <= scriptCount; ++i) {
        auto data = scripts->getScriptData<dedupType>(i);
        SHA256_Update(&sha256, &data->address, sizeof(uint160));
        SHA256_Update(&sha256, &data->pubkey, sizeof(RawPubkey));
        SHA256_Update(&sha256, &data->hasPubkey, sizeof(bool));
        SHA256_Update(&sha256, &data->txFirstSeen, 4);
        SHA256_Update(&sha256, &data->txFirstSpent, 4);
        SHA256_Update(&sha256, &data->typesSeen, 4);
    }

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
Compute a checksum over scripthash data.
*/
template<>
uint256 compute_scriptdata_hash<DedupAddressType::Enum::SCRIPTHASH>(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    auto scripts = &access.getScripts();
    constexpr DedupAddressType::Enum dedupType = DedupAddressType::SCRIPTHASH;
    auto scriptCount = scripts->scriptCount(dedupType);

    for(uint32_t i = 1; i <= scriptCount; ++i) {
        auto data = scripts->getScriptData<dedupType>(i);
        SHA256_Update(&sha256, &data->hash160, sizeof(uint160));
        SHA256_Update(&sha256, &data->hash256, sizeof(uint256));
        SHA256_Update(&sha256, &data->isSegwit, sizeof(bool));
        SHA256_Update(&sha256, &data->txFirstSeen, 4);
        SHA256_Update(&sha256, &data->txFirstSpent, 4);
        SHA256_Update(&sha256, &data->typesSeen, 4);
        SHA256_Update(&sha256, &data->wrappedAddress, sizeof(RawAddress));
    }

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
Compute a checksum over multisig data.
*/
template<>
uint256 compute_scriptdata_hash<DedupAddressType::Enum::MULTISIG>(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    auto scripts = &access.getScripts();
    constexpr DedupAddressType::Enum dedupType = DedupAddressType::MULTISIG;
    auto scriptCount = scripts->scriptCount(dedupType);

    for(uint32_t i = 1; i <= scriptCount; ++i) {
        auto data = scripts->getScriptData<dedupType>(i);
        SHA256_Update(&sha256, data, data->realSize());
    }

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
Compute a checksum over OP_RETURN data.
*/
template<>
uint256 compute_scriptdata_hash<DedupAddressType::Enum::NULL_DATA>(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    auto scripts = &access.getScripts();
    constexpr DedupAddressType::Enum dedupType = DedupAddressType::NULL_DATA;
    auto scriptCount = scripts->scriptCount(dedupType);

    for(uint32_t i = 1; i <= scriptCount; ++i) {
        auto data = scripts->getScriptData<dedupType>(i);
        SHA256_Update(&sha256, data, data->realSize());
    }

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
Compute a checksum over nonstandard data.
*/
template<>
uint256 compute_scriptdata_hash<DedupAddressType::Enum::NONSTANDARD>(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    auto scripts = &access.getScripts();
    constexpr DedupAddressType::Enum dedupType = DedupAddressType::NONSTANDARD;
    auto scriptCount = scripts->scriptCount(dedupType);

    for(uint32_t i = 1; i <= scriptCount; ++i) {
        auto data = scripts->getScriptData<dedupType>(i);
        auto script_data = std::get<0>(data);
        SHA256_Update(&sha256, script_data, script_data->realSize());
        auto spend_script_data = std::get<1>(data);
        if(spend_script_data != nullptr) {
            SHA256_Update(&sha256, spend_script_data, spend_script_data->realSize());
        }
    }

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
Compute a checksum over unknown witness data.
*/
template<>
uint256 compute_scriptdata_hash<DedupAddressType::Enum::WITNESS_UNKNOWN>(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    auto scripts = &access.getScripts();
    constexpr DedupAddressType::Enum dedupType = DedupAddressType::WITNESS_UNKNOWN;
    auto scriptCount = scripts->scriptCount(dedupType);

    for(uint32_t i = 1; i <= scriptCount; ++i) {
        auto data = scripts->getScriptData<dedupType>(i);
        auto script_data = std::get<0>(data);
        SHA256_Update(&sha256, &script_data->witnessVersion, sizeof(script_data->witnessVersion));
        SHA256_Update(&sha256, &script_data->scriptData, sizeof(script_data->scriptData) + script_data->scriptData.extraSize());
        auto spend_script_data = std::get<1>(data);
        if(spend_script_data != nullptr) {
            SHA256_Update(&sha256, spend_script_data, spend_script_data->realSize());
        }
    }

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
 Compute a checksum over address range in hash index
 */
template<AddressType::Enum type>
void hash_addressrange(SHA256_CTX &sha256, const DataAccess &access) {
    auto &hashIndex = access.hashIndex;
    auto rng = hashIndex->getAddressRange<type>();
    RANGES_FOR(auto pair, rng) {
        SHA256_Update(&sha256, &pair.first, sizeof(pair.first));
        SHA256_Update(&sha256, &pair.second, sizeof(pair.second));
    }
}

/**
 Compute a checksum over all addres ranges in hash index.
 */
uint256 compute_hashindex_addressrange_hash(const DataAccess &access) {
    uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    hash_addressrange<AddressType::MULTISIG>(sha256, access);
    hash_addressrange<AddressType::MULTISIG_PUBKEY>(sha256, access);
    hash_addressrange<AddressType::PUBKEY>(sha256, access);
    hash_addressrange<AddressType::PUBKEYHASH>(sha256, access);
    hash_addressrange<AddressType::SCRIPTHASH>(sha256, access);
    hash_addressrange<AddressType::WITNESS_PUBKEYHASH>(sha256, access);
    hash_addressrange<AddressType::WITNESS_SCRIPTHASH>(sha256, access);

    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

/**
 Compute a checksum over txindex lookup data in hash index.
 */
bool check_hashindex_txindex(const DataAccess &access) {
    const ChainAccess &chainAccess = access.getChain();
    auto chainDirectory = access.config.chainDirectory();
    FixedSizeFileMapper<uint256> txHashesFile(chainAccess.txHashesFilePath(chainDirectory));
    auto &hashIndex = access.hashIndex;

    bool allIndexesCorrect = true;

    for(uint32_t i = 0; i < chainAccess.txCount(); ++i) {
        auto txindex = hashIndex->getTxIndex(*txHashesFile[i]);
        if(*txindex != i) {
            allIndexesCorrect = false;
            std::cout << "Incorrect index for transaction " << i << ". Got: " << *txindex << "." << std::endl;
        }
    }
    if(allIndexesCorrect) {
        std::cout << "All txindex lookups correct for transaction hashes." << std::endl;
    }
    return allIndexesCorrect;
}


/**
 Check that we can resolve P2(W)SH addresses from the address they wrap using the address index
 */
bool check_nesting_scripthash_index(const DataAccess &access) {
    auto scripts = &access.getScripts();
    constexpr DedupAddressType::Enum dedupType = DedupAddressType::SCRIPTHASH;
    auto scriptCount = scripts->scriptCount(dedupType);
    auto &addressIndex = access.addressIndex;

    bool allNestingsCorrect = true;

    for(uint32_t i = 1; i <= scriptCount; ++i) {
        auto data = scripts->getScriptData<dedupType>(i);
        if(data->hasWrappedAddress()) {
            RawAddress wrappedAddress = data->wrappedAddress;
            DedupAddress expectedAddress{i, dedupType};
            auto nestingAddresses = addressIndex->getNestingScriptHash(wrappedAddress);

            // nesting of multisig addresses might not be unique since keys can appear in arbitrary order
            if(nestingAddresses.size() > 0) {
                if(std::find(nestingAddresses.begin(), nestingAddresses.end(), expectedAddress) == nestingAddresses.end()) {
                    allNestingsCorrect = false;
                    std::cout << "Incorrect nesting scripthash for (" << wrappedAddress.scriptNum << ", " << wrappedAddress.type << ")." << std::endl;
                }
            } else {
                allNestingsCorrect = false;
                std::cout << "Found no nesting scripthash for (" << wrappedAddress.scriptNum << ", " << wrappedAddress.type << "). Expected: (" << i << ", " << dedupType << ")." << std::endl;
            }
        }
    }
    if(allNestingsCorrect) {
        std::cout << "All reverse nested lookups correct for wrapped addresses." << std::endl;
    }
    return allNestingsCorrect;
}

int main(int argc, char * argv[]) {
    std::string configLocation;
    std::string outputFile;
    std::ofstream out;
    bool runIndexTests = false;
    std::streambuf *coutbuf = nullptr;
    int endBlock = 0;

    auto cli = (
        clipp::value("config file location", configLocation) % "Path to config file",
        (clipp::option("--file", "-f") & clipp::value("output file", outputFile)) % "Write to file instead of std::cout",
        clipp::option("--index", "-i").set(runIndexTests).doc("Run additional index tests")
    );

    auto res = parse(argc, argv, cli);
    if (res.any_error()) {
        std::cout << "Invalid command line parameter\n" << clipp::make_man_page(cli, argv[0]);
        return 0;
    }

    // Write to file instead of stdout
    if(!outputFile.empty()) {
        out.open(outputFile);
        coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(out.rdbuf());
    }

    Blockchain chain(configLocation, endBlock);

    const DataAccess &dataAccess = chain.getAccess();
    const ChainAccess &chainAccess = dataAccess.getChain();

    std::cout << "Chain contains " << chain.size() << " blocks, " << chain.getAccess().getChain().txCount() << " txes, " << chain.getAccess().getChain().inputCount() << " inputs, " << chain.getAccess().getChain().outputCount() << " outputs." << std::endl;

    std::cout << std::endl << "Blocks:" << std::endl;
    auto block_hash = compute_block_hash(chainAccess);
    std::cout << block_hash.GetHex() << " (BLOCKS)" <<  std::endl;

    std::cout << std::endl << "Transactions:" << std::endl;
    auto txdata_hash = compute_txdata_hash(chainAccess);
    std::cout << txdata_hash.GetHex() << " (TXES)" << std::endl;

    std::cout << std::endl << "Additional data:" << std::endl;
    auto additional_data_hash = compute_additional_data_hash(dataAccess);
    std::cout << additional_data_hash.GetHex() << " (ADDITIONAL)" <<  std::endl;

    std::cout << std::endl << "Scripts:" << std::endl;

    auto scripthash_hash = compute_scriptdata_hash<DedupAddressType::SCRIPTHASH>(dataAccess);
    std::cout << scripthash_hash.GetHex() <<  " (SCRIPTHASH)" << std::endl;

    auto pubkey_hash = compute_scriptdata_hash<DedupAddressType::PUBKEY>(dataAccess);
    std::cout << pubkey_hash.GetHex() <<  " (PUBKEY)" << std::endl;

    auto multisig_hash = compute_scriptdata_hash<DedupAddressType::MULTISIG>(dataAccess);
    std::cout << multisig_hash.GetHex() <<  " (MULTISIG)" << std::endl;

    auto nulldata_hash = compute_scriptdata_hash<DedupAddressType::NULL_DATA>(dataAccess);
    std::cout << nulldata_hash.GetHex() <<  " (NULL_DATA)" << std::endl;

    auto witnessunknown_hash = compute_scriptdata_hash<DedupAddressType::WITNESS_UNKNOWN>(dataAccess);
    std::cout << witnessunknown_hash.GetHex() <<  " (WITNESS_UNKNOWN)" << std::endl;

    auto nonstandard_hash = compute_scriptdata_hash<DedupAddressType::NONSTANDARD>(dataAccess);
    std::cout << nonstandard_hash.GetHex() <<  " (NONSTANDARD)" << std::endl;

    std::cout << std::endl << "Hash index:" << std::endl;
    auto hashindex_addressrange_hash = compute_hashindex_addressrange_hash(dataAccess);
    std::cout << hashindex_addressrange_hash.GetHex() << " (ADDRESSINDEX)" <<  std::endl;

    if(runIndexTests) {
        std::cout << std::endl << "Index: transaction hash->transaction index:" << std::endl;
        check_hashindex_txindex(dataAccess);

        std::cout << std::endl << "Index: nesting address->parent address:" << std::endl;
        check_nesting_scripthash_index(dataAccess);
    }

    if((!outputFile.empty()) && (coutbuf != nullptr)) {
        std::cout.rdbuf(coutbuf);
        out.close();
    }

    return 0;
}
