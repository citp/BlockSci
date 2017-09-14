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

BlockProcessor::BlockProcessor() : rawDone(false), utxoDone(false) {
    
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
    blocksci::FixedSizeFileMapper<blocksci::uint256, boost::iostreams::mapped_file::readwrite> hashFile{config.txHashesFilePath()};
    
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
        
        blocksci::uint256 nullHash;
        nullHash.SetNull();
        
        std::vector<unsigned char> coinbase;
        
        auto firstTxIndex = txNum;
        
        for (uint32_t i = 0; i < txCount; i++) {
            RawTransaction *tx;
            if (!finished_transaction_queue.pop(tx)) {
                tx = new RawTransaction();
            } else {
                auto it = files.begin();
                while (it != files.end()) {
                    if (it->second.second < tx->txNum) {
                        it->second.first.close();
                        it = files.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            tx->load(&startPos, block.height);
            
            hashFile.write(tx->hash);
            sequenceFile.writeIndexGroup();
            for (auto &input : tx->inputs) {
                sequenceFile.write(input.sequenceNum);
            }
            
            if (tx->inputs.size() == 1 && tx->inputs[0].rawOutputPointer.hash == nullHash) {
                auto scriptBegin = tx->inputs[0].scriptBegin;
                coinbase.assign(scriptBegin, scriptBegin + tx->inputs[0].scriptLength);
                tx->inputs.clear();
            }
            
            while (!utxo_transaction_queue.push(tx)) {
                std::this_thread::sleep_for(100ms);
            }
            txNum++;
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

void BlockProcessor::loadTxRPC(RawTransaction *tx, const blockinfo_t &block, uint32_t txNum, BitcoinAPI & bapi) {
    if (block.height == 0) {
        tx->outputs.clear();
        tx->outputs.reserve(1);
        
        auto scriptPubKey = CScript() << ParseHex("040184a11fa689ad5123690c81a3a49c8f13f8d45bac857fbcbc8bc4a8ead3eb4b1ff4d4614fa18dce611aaf1f471216fe1b51851b4acf21b17fc45171ac7b13af") << OP_CHECKSIG;
        std::vector<unsigned char> scriptBytes(scriptPubKey.begin(), scriptPubKey.end());
        //Set the desired initial block reward
        tx->outputs.emplace_back(scriptBytes, 50 * 100000000.0);
        tx->hash = blocksci::uint256S("0100000000000000000000000000000000000000000000000000000000000000");
        tx->blockHeight = 0;
    } else {
        auto txinfo = bapi.getrawtransaction(block.tx[txNum], 1);
        tx->load(txinfo, block.height);
    }
}

void BlockProcessor::readNewBlocks(RPCParserConfiguration config, std::vector<blockinfo_t> blocksToAdd, uint32_t startingTxCount) {
    using namespace std::chrono_literals;
    auto txNum = startingTxCount;
    
    blocksci::ArbitraryFileMapper<boost::iostreams::mapped_file::readwrite> blockCoinbaseFile(config.blockCoinbaseFilePath());
    blocksci::FixedSizeFileMapper<blocksci::Block, boost::iostreams::mapped_file::readwrite> blockFile(config.blockFilePath());
    blocksci::FixedSizeFileMapper<blocksci::uint256, boost::iostreams::mapped_file::readwrite> hashFile{config.txHashesFilePath()};
    blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, uint32_t> sequenceFile{config.sequenceFilePath()};
    
    BitcoinAPI bapi{config.createBitcoinAPI()};
    
    for (auto &block : blocksToAdd) {
        uint32_t blockTxCount = block.tx.size();
        
        std::vector<unsigned char> coinbase;
        auto firstTxIndex = txNum;
        for (uint32_t i = 0; i < blockTxCount; i++) {
            RawTransaction *tx;
            if (!finished_transaction_queue.pop(tx)) {
                tx = new RawTransaction();
            }
            loadTxRPC(tx, block, i, bapi);
            
            hashFile.write(tx->hash);
            sequenceFile.writeIndexGroup();
            for (auto &input : tx->inputs) {
                sequenceFile.write(input.sequenceNum);
            }
            
            // Note to self: Currently this does not get the coinbase tx data
            while (!utxo_transaction_queue.push(tx)) {
                std::this_thread::sleep_for(100ms);
            }
            txNum++;
        }
        
        blockFile.write(getBlock(firstTxIndex, blockTxCount, blockCoinbaseFile.size(), block));
        blockCoinbaseFile.write(coinbase.begin(), coinbase.end());
        
        firstTxIndex += blockTxCount;
    }
    
    rawDone = true;
}

#endif

struct ProcessOutputVisitor : public boost::static_visitor<blocksci::Address> {
    AddressState &state;
    AddressWriter &addressWriter;
    ProcessOutputVisitor(AddressState &state_, AddressWriter &addressWriter_) : state(state_), addressWriter(addressWriter_) {}
    template <blocksci::AddressType::Enum type>
    blocksci::Address operator()(ScriptOutput<type> &scriptOutput) const {
        std::pair<blocksci::Address, bool> processed = getAddressNum(scriptOutput, state);
        if (processed.second) {
            scriptOutput.processOutput(state);
            addressWriter.serialize(scriptOutput);
        }
        return processed.first;
    }
};

template<blocksci::AddressType::Enum type>
struct ScriptInputFunctor {
    static void f(uint32_t addressNum, const InputInfo &info, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter) {
        auto input = ScriptInput<type>(info, tx, addressWriter);
        input.processInput(addressNum, info, tx, state, addressWriter);
    }
};

void processInputVisitor(const blocksci::Address &address, const InputInfo &info, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter) {

    static constexpr auto table = blocksci::make_dynamic_table<blocksci::AddressType, ScriptInputFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(address.type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](address.addressNum, info, tx, state, addressWriter);
}

void BlockProcessor::processUTXOs(ParserConfiguration config, UTXOState &utxoState) {
    using namespace std::chrono_literals;
    
    std::cout.setf(std::ios::fixed,std::ios::floatfield);
    std::cout.precision(1);
    
    IndexedFileWriter<1> txFile{config.txFilePath()};
    
    auto txNum = txFile.size();
    
    auto consume = [&](RawTransaction *tx) -> void {
        
        tx->txNum = txNum;
        txFile.writeIndexGroup();
        txFile.write(tx->getRawTransaction());
        
        uint16_t i = 0;
        for (auto &output : tx->outputs) {
            auto type = addressType(output.scriptOutput);
            blocksci::Address address{0, type};
            
            blocksci::Inout blocksciOutput{0, address, output.value};
            txFile.write(blocksciOutput);
            
            if (blocksci::isSpendable(type)) {
                blocksciOutput.linkedTxNum = txNum;
                UTXO utxo{blocksciOutput, type};
                RawOutputPointer pointer{tx->hash, i};
                utxoState.addOutput(utxo, pointer);
            }
            i++;
        }
        
        i = 0;
        for (auto &input : tx->inputs) {
            auto utxo = utxoState.spendOutput(input.rawOutputPointer);
            input.addressType = utxo.addressType;
            input.linkedTxNum = utxo.output.linkedTxNum;
            i++;
            
            blocksci::Address address{0, utxo.addressType};
            blocksci::Inout blocksciInput{utxo.output.linkedTxNum, address, utxo.output.getValue()};
            txFile.write(blocksciInput);
        }
        
        
        
        bool flushed = false;
        while (!address_transaction_queue.push(tx)) {
            if (!flushed) {
                txFile.flush();
                flushed = true;
            }
            std::this_thread::sleep_for(100ms);
        }
        
        txNum++;
        
        utxoState.optionalSave();
    };
    
    while (!rawDone) {
        while (utxo_transaction_queue.consume_all(consume)) {}
        std::this_thread::sleep_for(100ms);
    }
    while (utxo_transaction_queue.consume_all(consume)) {}
    
    utxoDone = true;
}

void BlockProcessor::processAddresses(ParserConfiguration config, AddressState &addressState, uint32_t currentCount, uint32_t totalTxCount) {
    using namespace std::chrono_literals;
    
    using TxFile = blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction>;
    
    auto percentage = static_cast<double>(totalTxCount) / 1000.0;
    
    uint32_t percentageMarker = static_cast<uint32_t>(std::ceil(percentage));
    
    AddressWriter addressWriter{config};
    ProcessOutputVisitor outputVisitor{addressState, addressWriter};
    
    std::cout.setf(std::ios::fixed,std::ios::floatfield);
    std::cout.precision(1);
    
    ECCVerifyHandle handle;
    
    auto consume = [&](RawTransaction *tx, TxFile &txFile) -> void {
        auto diskTx = txFile.getData(tx->txNum);
        
        assert(diskTx != nullptr);
        assert(diskTx->inputCount == tx->inputs.size());
        assert(diskTx->outputCount == tx->outputs.size());
        
        uint16_t i = 0;
        for (auto &output : tx->outputs) {
            auto &diskOutput = diskTx->getOutput(i);
            auto address = boost::apply_visitor(outputVisitor, output.scriptOutput);
            assert(address.addressNum > 0);
            diskOutput.toAddressNum = address.addressNum;
            i++;
        }
        
        i = 0;
        for (auto &input : tx->inputs) {
            auto spentTx = txFile.getData(input.linkedTxNum);
            auto &spentOutput = spentTx->getOutput(input.rawOutputPointer.outputNum);
            
            assert(spentOutput.toAddressNum > 0);
            
            auto address = spentOutput.getAddress();
            auto &diskInput = diskTx->getInput(i);
            diskInput.toAddressNum = address.addressNum;
            spentOutput.linkedTxNum = tx->txNum;
            
            InputInfo info = input.getInfo(i);
            processInputVisitor(address, info, *tx, addressState, addressWriter);
            
            i++;
        }
        
        if (tx->sizeBytes > 800) {
            delete tx;
        } else {
            if (!finished_transaction_queue.push(tx)) {
                delete tx;
            }
        }
        
        currentCount++;
        
        if (currentCount % percentageMarker == 0) {
            std::cout << "\r" << (static_cast<double>(currentCount) / static_cast<double>(totalTxCount)) * 100 << "% done" << std::flush;
        }
        
        addressState.optionalSave();
    };
    
    TxFile txFile{config.txFilePath()};
    RawTransaction *rawTx = nullptr;
    while (!utxoDone) {
        while (address_transaction_queue.write_available() < 10000 && address_transaction_queue.pop(rawTx)) {
            
            if (rawTx->txNum + 10000 >= txFile.size()) {
                txFile = TxFile{config.txFilePath()};
            }
            
            consume(rawTx, txFile);
        }
        std::this_thread::sleep_for(100ms);
    }
    
    txFile = TxFile{config.txFilePath()};
    while (address_transaction_queue.pop(rawTx)) {
        consume(rawTx, txFile);
    }
}


BlockProcessor::~BlockProcessor() {
    assert(!utxo_transaction_queue.pop());
    assert(!address_transaction_queue.pop());
    
    finished_transaction_queue.consume_all([](RawTransaction *tx) {
        delete tx;
    });
}
