//
//  block_processor.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/11/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "block_processor.hpp"
#include "chain_writer.hpp"
#include "address_writer.hpp"
#include "script_input.hpp"

#include "utilities.hpp"
#include "preproccessed_block.hpp"

#include "chain_index.hpp"
#include "blockchain_state.hpp"

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

BlockProcessor::BlockProcessor() : done(false) {
    
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
    
    auto firstTxIndex = startingTxCount;
    
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
        
        for (uint32_t i = 0; i < txCount; i++) {
            RawTransaction *tx;
            if (!used_transaction_queue.pop(tx)) {
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
            
            tx->load(&startPos);
            
            if (tx->inputs.size() == 1 && tx->inputs[0].rawOutputPointer.hash == nullHash) {
                coinbase.assign(tx->inputs[0].scriptBegin, tx->inputs[0].scriptEnd);
                tx->inputs.clear();
            }
            
            while (!transaction_queue.push(tx)) {
                std::this_thread::sleep_for(100ms);
            }
        }
        
        blockFile.write(getBlock(firstTxIndex, txCount, blockCoinbaseFile.size(), block));
        blockCoinbaseFile.write(coinbase.begin(), coinbase.end());
        
        firstTxIndex += txCount;
    }
    done = true;
}

#endif

#ifdef BLOCKSCI_RPC_PARSER

blocksci::Block getBlock(uint32_t firstTxIndex, uint32_t txCount, size_t coinbasePos, const blockinfo_t &block) {
    return {firstTxIndex, txCount, static_cast<uint32_t>(block.height), blocksci::uint256S(block.hash), block.version, block.time, static_cast<uint32_t>(std::stoul(block.bits, nullptr, 16)), block.nonce, coinbasePos};
}

void BlockProcessor::loadTxRPC(RawTransaction *tx, const blockinfo_t &block, uint32_t txNum, BitcoinAPI & bapi) {
    if (block.height == 0) {
        tx->hash = blocksci::uint256S("0100000000000000000000000000000000000000000000000000000000000000");
        tx->outputs.clear();
        tx->outputs.reserve(1);
        
        auto scriptPubKey = CScript() << ParseHex("040184a11fa689ad5123690c81a3a49c8f13f8d45bac857fbcbc8bc4a8ead3eb4b1ff4d4614fa18dce611aaf1f471216fe1b51851b4acf21b17fc45171ac7b13af") << OP_CHECKSIG;
        std::vector<unsigned char> scriptBytes(scriptPubKey.begin(), scriptPubKey.end());
        //Set the desired initial block reward
        tx->outputs.emplace_back(scriptBytes, 50 * 100000000.0);
    } else {
        auto txinfo = bapi.getrawtransaction(block.tx[txNum], 1);
        tx->load(txinfo);
    }
}

void BlockProcessor::readNewBlocks(RPCParserConfiguration config, std::vector<blockinfo_t> blocksToAdd, uint32_t startingTxCount) {
    using namespace std::chrono_literals;
    auto firstTxIndex = startingTxCount;
    
    blocksci::ArbitraryFileMapper<boost::iostreams::mapped_file::readwrite> blockCoinbaseFile(config.blockCoinbaseFilePath());
    blocksci::FixedSizeFileMapper<blocksci::Block, boost::iostreams::mapped_file::readwrite> blockFile(config.blockFilePath());
    
    BitcoinAPI bapi{config.createBitcoinAPI()};
    
    for (auto &block : blocksToAdd) {
        uint32_t blockTxCount = block.tx.size();
        
        std::vector<unsigned char> coinbase;
        
        for (uint32_t i = 0; i < blockTxCount; i++) {
            RawTransaction *tx;
            if (!used_transaction_queue.pop(tx)) {
                tx = new RawTransaction();
            }
            loadTxRPC(tx, block, i, bapi);
            
            // Note to self: Currently this does not get the coinbase tx data
            while (!transaction_queue.push(tx)) {
                std::this_thread::sleep_for(100ms);
            }
        }
        
        blockFile.write(getBlock(firstTxIndex, blockTxCount, blockCoinbaseFile.size(), block));
        blockCoinbaseFile.write(coinbase.begin(), coinbase.end());
        
        firstTxIndex += blockTxCount;
    }
    
    done = true;
}

#endif

struct ProcessOutputVisitor : public boost::static_visitor<blocksci::Address> {
    BlockchainState &state;
    AddressWriter &addressWriter;
    ProcessOutputVisitor(BlockchainState &state_, AddressWriter &addressWriter_) : state(state_), addressWriter(addressWriter_) {}
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
    static void f(const InputInfo &info, const RawTransaction &tx, BlockchainState &state, AddressWriter &addressWriter) {
        auto input = ScriptInput<type>(info, tx, addressWriter);
        input.processInput(info, tx, state, addressWriter);
    }
};

void processInputVisitor(const InputInfo &info, const RawTransaction &tx, BlockchainState &state, AddressWriter &addressWriter) {
    auto &type = info.address.type;
    
    static constexpr auto table = blocksci::make_dynamic_table<ScriptInputFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](info, tx, state, addressWriter);
}

void BlockProcessor::processNewBlocks(ParserConfiguration config, uint32_t firstTxNum, uint32_t totalTxCount) {
    using namespace std::chrono_literals;
    
    ChainWriter writer(config);
    AddressWriter addressWriter(config);
    uint32_t txNum = firstTxNum;
    
    auto percentage = static_cast<double>(totalTxCount) / 1000.0;
    
    uint32_t percentageMarker = static_cast<uint32_t>(std::ceil(percentage));
    
    BlockchainState state(config);
    
    ProcessOutputVisitor outputVisitor{state, addressWriter};
    
    std::cout.setf(std::ios::fixed,std::ios::floatfield);
    std::cout.precision(1);
    
    uint32_t count = 0;
    
    ECCVerifyHandle handle;
    
    blocksci::FixedSizeFileMapper<TxUpdate, boost::iostreams::mapped_file::readwrite> txUpdateFile(config.txUpdatesFilePath());
    
    auto consume = [&](RawTransaction *tx) -> void {
        
        writer.writeTransactionHeader(tx->getRawTransaction());
        writer.writeTransactionHash(tx->hash);
        
        tx->txNum = txNum;
        
        uint16_t i = 0;
        for (auto &output : tx->outputs) {
            auto address = boost::apply_visitor(outputVisitor, output.scriptOutput);
            
            blocksci::Inout blocksciOutput{0, address, output.value};
            writer.writeTransactionOutput(blocksciOutput);
            
            if (address.isSpendable()) {
                blocksciOutput.linkedTxNum = txNum;
                UTXO utxo{blocksciOutput, address};
                RawOutputPointer pointer{tx->hash, i};
                state.addOutput(utxo, pointer);
            }
            i++;
        }
        
        i = 0;
        for (auto &input : tx->inputs) {
            auto utxo = state.spendOutput(input.rawOutputPointer);
            txUpdateFile.write({{utxo.output.linkedTxNum, input.rawOutputPointer.outputNum}, txNum});
            // This is called for every input since there is currently no way to detect the first input spending an address. Additionally an address may be spent in different ways (ex. multisig)
            InputInfo info = input.getInfo(utxo, i);
            processInputVisitor(info, *tx, state, addressWriter);
            writer.writeTransactionInput(utxo.output, input.sequenceNum);
            i++;
        }
        
        if (tx->sizeBytes > 800) {
            delete tx;
        } else {
            if (!used_transaction_queue.push(tx)) {
                delete tx;
            }
        }
        
        txNum++;
        count++;
        
        if (count % percentageMarker == 0) {
            std::cout << "\r" << (static_cast<double>(count) / static_cast<double>(totalTxCount)) * 100 << "% done" << std::flush;
        }
        
        state.optionalSave();
    };
    
    while (!done) {
        while (transaction_queue.consume_all(consume)) {}
        std::this_thread::sleep_for(100ms);
    }
    while (transaction_queue.consume_all(consume)) {}
    std::cout << "\nDone processing txes\n";
}

BlockProcessor::~BlockProcessor() {
    used_transaction_queue.consume_all([](RawTransaction *tx) {
        delete tx;
    });
}
