//
//  block_processor.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/11/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "block_processor.hpp"
#include "script_input.hpp"
#include "file_writer.hpp"

#include "utilities.hpp"
#include "preproccessed_block.hpp"

#include "chain_index.hpp"
#include "utxo_state.hpp"
#include "address_state.hpp"

#include <blocksci/hash.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/bitcoin_pubkey.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <thread>
#include <fstream>
#include <iostream>

BlockProcessor::BlockProcessor() : rawDone(false), hashDone(false), utxoDone(false) {
    
}

const signed char p_util_hexdigit[256] =
{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
    -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, };

signed char HexDigit(char c)
{
    return p_util_hexdigit[(unsigned char)c];
}

std::vector<unsigned char> ParseHex(const char* psz)
{
    // convert hex dump to vector
    std::vector<unsigned char> vch;
    while (true)
    {
        while (isspace(*psz))
            psz++;
        signed char c = HexDigit(*psz++);
        if (c == (signed char)-1)
            break;
        unsigned char n = (c << 4);
        c = HexDigit(*psz++);
        if (c == (signed char)-1)
            break;
        n |= c;
        vch.push_back(n);
    }
    return vch;
}

#ifdef BLOCKSCI_FILE_PARSER

blocksci::Block getBlock(uint32_t firstTxIndex, uint32_t txCount, size_t coinbasePos, const BlockInfo &block) {
    return {firstTxIndex, txCount, static_cast<uint32_t>(block.height), block.hash, block.nVersion, block.nTime, block.nBits, block.nNonce, coinbasePos};
}


void BlockProcessor::closeFinishedFiles(uint32_t txNum) {
    auto it = files.begin();
    while (it != files.end()) {
        if (it->second.second < txNum) {
            it->second.first.close();
            it = files.erase(it);
        } else {
            ++it;
        }
    }
}

struct SegwitChecker : public boost::static_visitor<bool> {
    bool operator()(const ScriptOutput<blocksci::AddressType::Enum::NULL_DATA> &output) const {
        uint32_t segwitMarker = *reinterpret_cast<const uint32_t *>(output.fullData.data());
        return segwitMarker == 0xeda921aa;
    }
    
    template <blocksci::AddressType::Enum type>
    bool operator()(const ScriptOutput<type> &) const {
        return false;
    }
};

bool checkSegwit(RawTransaction *tx, const SegwitChecker &checker) {
    for (int i = tx->outputs.size() - 1; i >= 0; i--) {
        if (boost::apply_visitor(checker, tx->outputs[i].scriptOutput)) {
            return true;
        }
    }
    return false;
}

void BlockProcessor::readNewBlocks(FileParserConfiguration config, std::vector<BlockInfo> blocksToAdd, uint32_t startingTxCount) {
    using namespace std::chrono_literals;
    
    boost::unordered_map<int, uint32_t> firstTimeRequired;
    boost::unordered_map<int, uint32_t> lastBlockRequired;
    boost::unordered_map<int, uint32_t> lastTxRequired;
    
    auto txNum = startingTxCount;
    
    int maxBlockFile = 0;
    int minBlockFile = std::numeric_limits<int>::max();
    for (auto &block : blocksToAdd) {
        startingTxCount += block.nTx;
        firstTimeRequired.insert(std::make_pair(block.nFile, block.height));
        lastBlockRequired[block.nFile] = block.height;
        lastTxRequired[block.nFile] = startingTxCount;
        minBlockFile = std::min(block.nFile, minBlockFile);
        maxBlockFile = std::max(block.nFile, maxBlockFile);
    }
    
    blocksci::ArbitraryFileMapper<boost::iostreams::mapped_file::readwrite> blockCoinbaseFile(config.blockCoinbaseFilePath());
    blocksci::FixedSizeFileMapper<blocksci::Block, boost::iostreams::mapped_file::readwrite> blockFile(config.blockFilePath());
    blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, uint32_t> sequenceFile{config.sequenceFilePath()};
    
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    
    SegwitChecker checker;
    
    std::vector<unsigned char> coinbase;
    
    for (uint32_t i = 0; i < blocksToAdd.size(); i++) {
        auto &block = blocksToAdd[i];
        
        auto fileIt = files.find(block.nFile);
        if (fileIt == files.end()) {
            auto blockPath = config.pathForBlockFile(block.nFile);
            boost::iostreams::mapped_file blockFile(blockPath, boost::iostreams::mapped_file::readonly);
            if (!blockFile.is_open()) {
                std::cout << "Error: Failed to open block file " << blockPath << "\n";
                break;
            }
            auto mappedPair = std::make_pair(boost::iostreams::mapped_file(blockPath, boost::iostreams::mapped_file::readonly), lastTxRequired[block.nFile]);
            files[block.nFile] = mappedPair;
        }
        
        const char *startPos = files[block.nFile].first.const_data() + block.nDataPos;
        
        constexpr size_t blockHeaderSize = 80;
        startPos += blockHeaderSize;
        uint32_t txCount = readVariableLengthInteger(&startPos);
        
        auto firstTxIndex = txNum;
        
        RawTransaction *tx;
        if (!finished_transaction_queue.pop(tx)) {
            tx = new RawTransaction();
        } else {
            closeFinishedFiles(tx->txNum);
        }
        
        blocksci::uint256 nullHash;
        nullHash.SetNull();
        bool segwit = false;
        auto segwitPos = startPos;
        for (uint32_t i = 0; i < txCount; i++) {
            tx->load(&segwitPos, 0, false);
            if (tx->inputs.size() == 1 && tx->inputs[0].rawOutputPointer.hash == nullHash) {
                segwit = checkSegwit(tx, checker);
                break;
            }
        }
        
        for (uint32_t i = 0; i < txCount; i++) {
            tx->load(&startPos, block.height, segwit);
            tx->txNum = txNum;
            
            sequenceFile.writeIndexGroup();
            for (auto &input : tx->inputs) {
                sequenceFile.write(input.sequenceNum);
            }
            
            if (tx->inputs.size() == 1 && tx->inputs[0].rawOutputPointer.hash == nullHash) {
                auto scriptBegin = tx->inputs[0].scriptBegin;
                coinbase.assign(scriptBegin, scriptBegin + tx->inputs[0].scriptLength);
                tx->inputs.clear();
            }
            
            while (!hash_transaction_queue.push(tx)) {
                std::this_thread::sleep_for(100ms);
            }
            txNum++;
            
            if (!finished_transaction_queue.pop(tx)) {
                tx = new RawTransaction();
            } else {
                closeFinishedFiles(tx->txNum);
            }
        }
        
        blockFile.write(getBlock(firstTxIndex, txCount, blockCoinbaseFile.size(), block));
        blockCoinbaseFile.write(coinbase.begin(), coinbase.end());
    }
    rawDone = true;
}

#endif

#ifdef BLOCKSCI_RPC_PARSER

blocksci::Block getBlock(uint32_t firstTxIndex, uint32_t txCount, size_t coinbasePos, const blockinfo_t &block) {
    return {firstTxIndex, txCount, static_cast<uint32_t>(block.height), blocksci::uint256S(block.hash), block.version, block.time, static_cast<uint32_t>(std::stoul(block.bits, nullptr, 16)), block.nonce, coinbasePos};
}

void BlockProcessor::loadTxRPC(RawTransaction *tx, const blockinfo_t &block, uint32_t txNum, BitcoinAPI & bapi, bool witnessActivated) {
    if (block.height == 0) {
        tx->outputs.clear();
        tx->outputs.reserve(1);
        
        auto scriptPubKey = CScript() << ParseHex("040184a11fa689ad5123690c81a3a49c8f13f8d45bac857fbcbc8bc4a8ead3eb4b1ff4d4614fa18dce611aaf1f471216fe1b51851b4acf21b17fc45171ac7b13af") << OP_CHECKSIG;
        std::vector<unsigned char> scriptBytes(scriptPubKey.begin(), scriptPubKey.end());
        //Set the desired initial block reward
        tx->outputs.emplace_back(scriptBytes, 50 * 100000000.0, false);
        tx->hash = blocksci::uint256S("0100000000000000000000000000000000000000000000000000000000000000");
        tx->blockHeight = 0;
    } else {
        auto txinfo = bapi.getrawtransaction(block.tx[txNum], 1);
        tx->load(txinfo, block.height, witnessActivated);
    }
}

void BlockProcessor::readNewBlocks(RPCParserConfiguration config, std::vector<blockinfo_t> blocksToAdd, uint32_t startingTxCount) {
    using namespace std::chrono_literals;
    auto txNum = startingTxCount;
    
    blocksci::ArbitraryFileMapper<boost::iostreams::mapped_file::readwrite> blockCoinbaseFile(config.blockCoinbaseFilePath());
    blocksci::FixedSizeFileMapper<blocksci::Block, boost::iostreams::mapped_file::readwrite> blockFile(config.blockFilePath());
    blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, uint32_t> sequenceFile{config.sequenceFilePath()};
    
    BitcoinAPI bapi{config.createBitcoinAPI()};
    
    
    RawTransaction *tx;
    SegwitChecker checker;
    
    for (auto &block : blocksToAdd) {
        uint32_t blockTxCount = block.tx.size();
        if (!finished_transaction_queue.pop(tx)) {
            tx = new RawTransaction();
        }
        
        loadTxRPC(tx, block, 0, bapi, false);
        bool segwit = checkSegwit(tx, checker);
        
        
        std::vector<unsigned char> coinbase;
        auto firstTxIndex = txNum;
        for (uint32_t i = 0; i < blockTxCount; i++) {
            
            if (!finished_transaction_queue.pop(tx)) {
                tx = new RawTransaction();
            }
            loadTxRPC(tx, block, i, bapi, segwit);
            tx->txNum = txNum;
            
            sequenceFile.writeIndexGroup();
            for (auto &input : tx->inputs) {
                sequenceFile.write(input.sequenceNum);
            }
            
            // Note to self: Currently this does not get the coinbase tx data
            while (!hash_transaction_queue.push(tx)) {
                std::this_thread::sleep_for(100ms);
            }
            txNum++;
        }
        
        blockFile.write(getBlock(firstTxIndex, blockTxCount, blockCoinbaseFile.size(), block));
        blockCoinbaseFile.write(coinbase.begin(), coinbase.end());
    }
    
    rawDone = true;
}

#endif

void BlockProcessor::calculateHashes(ParserConfiguration config) {
    using namespace std::chrono_literals;
    blocksci::FixedSizeFileMapper<blocksci::uint256, boost::iostreams::mapped_file::readwrite> hashFile{config.txHashesFilePath()};
    auto consume = [&](RawTransaction *tx) -> void {
        tx->calculateHash();
        hashFile.write(tx->hash);
        while (!utxo_transaction_queue.push(tx)) {
            std::this_thread::sleep_for(100ms);
        }
    };
    
    while (!rawDone) {
        while (hash_transaction_queue.consume_all(consume)) {}
        std::this_thread::sleep_for(100ms);
    }
    while (hash_transaction_queue.consume_all(consume)) {}
    
    hashDone = true;
}

void BlockProcessor::processUTXOs(ParserConfiguration config, UTXOState &utxoState) {
    using namespace std::chrono_literals;
    
    IndexedFileWriter<1> txFile{config.txFilePath()};
    
    auto consume = [&](RawTransaction *tx) -> void {
        
        txFile.writeIndexGroup();
        txFile.write(tx->getRawTransaction());
        
        uint16_t i = 0;
        
        for (auto &input : tx->inputs) {
            auto utxo = utxoState.spendOutput(input.rawOutputPointer);
            input.addressType = utxo.addressType;
            input.linkedTxNum = utxo.output.linkedTxNum;
            i++;
            
            blocksci::Address address{0, utxo.addressType};
            blocksci::Inout blocksciInput{utxo.output.linkedTxNum, address, utxo.output.getValue()};
            txFile.write(blocksciInput);
        }
        
        i = 0;
        for (auto &output : tx->outputs) {
            auto type = addressType(output.scriptOutput);
            blocksci::Address address{0, type};
            
            blocksci::Inout blocksciOutput{0, address, output.value};
            txFile.write(blocksciOutput);
            
            if (isSpendable(scriptType(type))) {
                blocksciOutput.linkedTxNum = tx->txNum;
                UTXO utxo{blocksciOutput, type};
                RawOutputPointer pointer{tx->hash, i};
                utxoState.addOutput(utxo, pointer);
            }
            i++;
        }
        
        
        bool flushed = false;
        while (!address_transaction_queue.push(tx)) {
            if (!flushed) {
                txFile.flush();
                flushed = true;
            }
            std::this_thread::sleep_for(100ms);
        }
        
        utxoState.optionalSave();
    };
    
    while (!hashDone) {
        while (utxo_transaction_queue.consume_all(consume)) {}
        std::this_thread::sleep_for(100ms);
    }
    while (utxo_transaction_queue.consume_all(consume)) {}
    
    utxoDone = true;
}

std::vector<uint32_t> BlockProcessor::processAddresses(ParserConfiguration config, AddressState &addressState, uint32_t currentCount, uint32_t totalTxCount, uint32_t maxBlockHeight) {
    using namespace std::chrono_literals;
    
    using TxFile = blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction>;
    
    auto percentage = static_cast<double>(totalTxCount) / 1000.0;
    
    uint32_t percentageMarker = static_cast<uint32_t>(std::ceil(percentage));
    
    AddressWriter addressWriter{config};
    
    std::cout.setf(std::ios::fixed,std::ios::floatfield);
    std::cout.precision(1);
    
    ECCVerifyHandle handle;
    
    std::vector<uint32_t> revealed;
    
    auto consume = [&](RawTransaction *tx, TxFile &txFile) -> void {
        auto diskTx = txFile.getData(tx->txNum);
        
        assert(diskTx != nullptr);
        assert(diskTx->inputCount == tx->inputs.size());
        assert(diskTx->outputCount == tx->outputs.size());
        
        uint16_t i = 0;
        
        for (auto &input : tx->inputs) {
            auto spentTx = txFile.getData(input.linkedTxNum);
            auto &spentOutput = spentTx->getOutput(input.rawOutputPointer.outputNum);
            
            assert(spentOutput.toAddressNum > 0);
            
            auto address = spentOutput.getAddress();
            auto &diskInput = diskTx->getInput(i);
            diskInput.toAddressNum = address.addressNum;
            spentOutput.linkedTxNum = tx->txNum;
            
            InputInfo info = input.getInfo(i, tx->txNum, address.addressNum, tx->isSegwit);
            auto processedInput = processInput(address, info, *tx, addressState, addressWriter);
            
            for (auto &index : processedInput) {
                revealed.push_back(index);
            }
            i++;
        }
        // 2,262,720
        i = 0;
        for (auto &output : tx->outputs) {
            auto &diskOutput = diskTx->getOutput(i);
            auto address = processOutput(output.scriptOutput, addressState, addressWriter);
            assert(address.addressNum > 0);
            diskOutput.toAddressNum = address.addressNum;
            i++;
        }
        
        currentCount++;
        
        if (currentCount % percentageMarker == 0) {
            auto percentDone = (static_cast<double>(currentCount) / static_cast<double>(totalTxCount)) * 100;
            auto blockHeight = tx->blockHeight;
            std::cout << "\r" << percentDone << "% done, Block " << blockHeight << "/" << maxBlockHeight << std::flush;
        }
        
        if (tx->sizeBytes > 800) {
            delete tx;
        } else {
            if (!finished_transaction_queue.push(tx)) {
                delete tx;
            }
        }
        
        addressState.optionalSave();
    };
    
    TxFile txFile{config.txFilePath()};
    RawTransaction *rawTx = nullptr;
    while (!utxoDone) {
        while (address_transaction_queue.write_available() < 10000 && address_transaction_queue.pop(rawTx)) {
            if (rawTx->txNum + 10000 >= txFile.size()) {
                txFile.reload();
            }
            consume(rawTx, txFile);
        }
        std::this_thread::sleep_for(100ms);
    }
    txFile.reload();
    while (address_transaction_queue.pop(rawTx)) {
        consume(rawTx, txFile);
    }
    
    return revealed;
}


BlockProcessor::~BlockProcessor() {
    assert(!utxo_transaction_queue.pop());
    assert(!address_transaction_queue.pop());
    
    finished_transaction_queue.consume_all([](RawTransaction *tx) {
        delete tx;
    });
}
