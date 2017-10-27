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
#include "address_writer.hpp"
#include "safe_mem_reader.hpp"
#include "chain_index.hpp"
#include "preproccessed_block.hpp"
#include "chain_index.hpp"
#include "utxo_state.hpp"
#include "address_state.hpp"

#include <blocksci/hash.hpp>
#include <blocksci/bitcoin_uint256.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/bitcoin_pubkey.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/atomic.hpp>
#include <boost/container/small_vector.hpp>

#include <gperftools/profiler.h>

#include <thread>
#include <fstream>
#include <iostream>

std::vector<unsigned char> ParseHex(const char* psz);

BlockProcessor::BlockProcessor(uint32_t startingTxCount_, uint32_t totalTxCount_, uint32_t maxBlockHeight_) : startingTxCount(startingTxCount_), currentTxNum(startingTxCount_), totalTxCount(totalTxCount_), maxBlockHeight(maxBlockHeight_) {
    
}

std::vector<unsigned char> ParseHex(const char* psz)
{
    // convert hex dump to vector
    std::vector<unsigned char> vch;
    while (true)
    {
        while (isspace(*psz))
            psz++;
        signed char c = blocksci::HexDigit(*psz++);
        if (c == static_cast<signed char>(-1))
            break;
        unsigned char n = static_cast<unsigned char>(c << 4);
        c = blocksci::HexDigit(*psz++);
        if (c == static_cast<signed char>(-1))
            break;
        n |= c;
        vch.push_back(n);
    }
    return vch;
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

bool checkSegwit(RawTransaction *tx) {
    SegwitChecker checker;
    for (int i = static_cast<int>(tx->outputs.size()) - 1; i >= 0; i--) {
        if (boost::apply_visitor(checker, tx->outputs[static_cast<size_t>(i)].scriptOutput)) {
            return true;
        }
    }
    return false;
}

template <typename ParseTag>
class BlockFileReader;

#ifdef BLOCKSCI_FILE_PARSER

template <>
class BlockFileReader<FileTag> : public BlockFileReaderBase {
    std::unordered_map<int, std::pair<SafeMemReader, uint32_t>> files;
    std::unordered_map<int, uint32_t> lastTxRequired;
    const ParserConfiguration<FileTag> &config;
    SafeMemReader *reader;
    
    uint32_t currentHeight;
    uint32_t currentTxNum;
    
    template<bool shouldAdvance>
    void nextTxImp(RawTransaction *tx, bool isSegwit) {
        try {
            auto firstTxOffset = reader->offset();
            tx->load(*reader, currentTxNum, currentHeight, isSegwit);
            if constexpr (shouldAdvance) {
                currentTxNum++;
            } else {
                reader->reset(firstTxOffset);
            }
        } catch (const std::exception &e) {
            std::cerr << "Failed to load tx"
            << " from block" << currentHeight
            << " at offset " << reader->offset()
            << ".\n";
            throw;
        }
    }
    
public:
    BlockFileReader(const ParserConfiguration<FileTag> &config_, std::vector<BlockInfo<FileTag>> &blocksToAdd, uint32_t firstTxNum) : config(config_) {
        for (auto &block : blocksToAdd) {
            firstTxNum += block.nTx;
            lastTxRequired[block.nFile] = firstTxNum;
        }
    }
    
    void nextBlock(BlockInfo<FileTag> &block, uint32_t firstTxNum) {
        auto fileIt = files.find(block.nFile);
        if (fileIt == files.end()) {
            auto blockPath = config.pathForBlockFile(block.nFile);
            if (!boost::filesystem::exists(blockPath)) {
                std::stringstream ss;
                ss << "Error: Failed to open block file " << blockPath << "\n";
                throw std::runtime_error(ss.str());
            }
            files.emplace(std::piecewise_construct, std::forward_as_tuple(block.nFile), std::forward_as_tuple(blockPath, lastTxRequired[block.nFile]));
        }
        reader = &files.at(block.nFile).first;
        reader->reset(block.nDataPos);
        reader->advance(sizeof(CBlockHeader));
        reader->readVariableLengthInteger();
        currentHeight = static_cast<uint32_t>(block.height);
        currentTxNum = firstTxNum;
        
    }
    
    void nextTx(RawTransaction *tx, bool isSegwit) override {
        nextTxImp<true>(tx, isSegwit);
    }
    
    void nextTxNoAdvance(RawTransaction *tx, bool isSegwit) override {
        nextTxImp<false>(tx, isSegwit);
    }
    
    void receivedFinishedTx(RawTransaction *tx) override {
        auto it = files.begin();
        while (it != files.end()) {
            if (it->second.second < tx->txNum) {
                it = files.erase(it);
            } else {
                ++it;
            }
        }
    }
};

#endif

#ifdef BLOCKSCI_RPC_PARSER

template <>
class BlockFileReader<RPCTag> : public BlockFileReaderBase {
    BitcoinAPI bapi;
    
    uint32_t firstTxNum;
    uint32_t currentTxOffset;
    uint32_t currentHeight;
    BlockInfo<RPCTag> block;
    
    template<bool shouldAdvance>
    void nextTxImp(RawTransaction *tx, bool isSegwit) {
        if (currentHeight == 0) {
            tx->outputs.clear();
            tx->outputs.reserve(1);
            
            auto scriptPubKey = CScript() << ParseHex("040184a11fa689ad5123690c81a3a49c8f13f8d45bac857fbcbc8bc4a8ead3eb4b1ff4d4614fa18dce611aaf1f471216fe1b51851b4acf21b17fc45171ac7b13af") << OP_CHECKSIG;
            std::vector<unsigned char> scriptBytes(scriptPubKey.begin(), scriptPubKey.end());
            //Set the desired initial block reward
            tx->outputs.emplace_back(scriptBytes, 50 * 100000000.0, false);
            tx->hash = blocksci::uint256S("0100000000000000000000000000000000000000000000000000000000000000");
            tx->blockHeight = 0;
            tx->txNum = 0;
        } else {
            auto txinfo = bapi.getrawtransaction(block.tx[currentTxOffset], 1);
            tx->load(txinfo, firstTxNum + currentTxOffset, currentHeight, isSegwit);
        }
        if constexpr (shouldAdvance) {
            currentTxOffset++;
        }
    }
    
public:
    BlockFileReader(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> &, uint32_t) : bapi(config.createBitcoinAPI()) {}
    
    void nextBlock(BlockInfo<RPCTag> &block_, uint32_t txNum) {
        block = block_;
        currentHeight = static_cast<uint32_t>(block.height);
        firstTxNum = txNum;
        currentTxOffset = 0;
    }
    
    void nextTx(RawTransaction *tx, bool isSegwit) override {
        nextTxImp<true>(tx, isSegwit);
    }
    
    void nextTxNoAdvance(RawTransaction *tx, bool isSegwit) override {
        nextTxImp<false>(tx, isSegwit);
    }
    
    void receivedFinishedTx(RawTransaction *) override {}
};

#endif

std::vector<unsigned char> readNewBlock(uint32_t firstTxNum, const BlockInfoBase &block, BlockFileReaderBase &fileReader, NewBlocksFiles &files, const std::function<bool(RawTransaction *&tx)> &loadFunc, const std::function<void(RawTransaction *tx)> &outFunc) {
    std::vector<unsigned char> coinbase;
    bool isSegwit = false;
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    for (uint32_t j = 0; j < block.nTx; j++) {
        RawTransaction *tx = nullptr;
        if (!loadFunc(tx)) {
            tx = new RawTransaction();
        } else {
            assert(tx);
            fileReader.receivedFinishedTx(tx);
        }
        
        if (j == 0) {
            fileReader.nextTxNoAdvance(tx, false);
            isSegwit = checkSegwit(tx);
        }
        
        fileReader.nextTx(tx, isSegwit);
        
        files.sequenceFile.writeIndexGroup();
        for (auto &input : tx->inputs) {
            files.sequenceFile.write(input.sequenceNum);
        }
        
        if (tx->inputs.size() == 1 && tx->inputs[0].rawOutputPointer.hash == nullHash) {
            auto scriptBegin = tx->inputs[0].getScriptBegin();
            coinbase.assign(scriptBegin, scriptBegin + tx->inputs[0].getScriptLength());
            tx->inputs.clear();
        }
        
        outFunc(tx);
    }
    blocksci::Block blocksciBlock{firstTxNum, block.nTx, static_cast<uint32_t>(block.height), block.hash, block.header.nVersion, block.header.nTime, block.header.nBits, block.header.nNonce, files.blockCoinbaseFile.size()};
    firstTxNum += block.nTx;
    files.blockFile.write(blocksciBlock);
    files.blockCoinbaseFile.write(coinbase.begin(), coinbase.end());
    
    return coinbase;
}

void calculateHash(RawTransaction *tx, FixedSizeFileWriter<blocksci::uint256> &hashFile) {
    tx->calculateHash();
    hashFile.write(tx->hash);
}

void connectUTXOs(RawTransaction *tx, UTXOState &utxoState) {
    for (auto &input : tx->inputs) {
        auto utxo = utxoState.spendOutput(input.rawOutputPointer);
        input.addressType = utxo.addressType;
        input.linkedTxNum = utxo.output.linkedTxNum;
        input.value = utxo.output.getValue();
    }
    
    uint16_t i = 0;
    for (auto &output : tx->outputs) {
        auto type = addressType(output.scriptOutput);
        if (isSpendable(scriptType(type))) {
            blocksci::Address address{0, type};
            blocksci::Inout blocksciOutput{tx->txNum, address, output.value};
            UTXO utxo{blocksciOutput, type};
            RawOutputPointer pointer{tx->hash, i};
            utxoState.addOutput(utxo, pointer);
        }
        i++;
    }
};

std::vector<uint32_t> connectAddressess(RawTransaction *tx, AddressState &addressState, AddressWriter &addressWriter, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile) {
    std::vector<uint32_t> revealed;
    
    txFile.writeIndexGroup();
    txFile.write(tx->getRawTransaction());
    
    uint16_t i = 0;
    for (auto &input : tx->inputs) {
        auto spentTx = txFile.getData(input.linkedTxNum);
        auto &spentOutput = spentTx->getOutput(input.rawOutputPointer.outputNum);
        auto address = spentOutput.getAddress();
        
        InputView inputView(i, tx->txNum, input.witnessStack, tx->isSegwit);
        InputScriptView scriptView(address.scriptNum, input.getScriptBegin(), input.getScriptLength());
        auto processedInput = processInput(address, inputView, scriptView, *tx, addressState, addressWriter);
        
        for (auto &index : processedInput) {
            revealed.push_back(index);
        }
        
        spentOutput.linkedTxNum = tx->txNum;
        blocksci::Inout blocksciInput{input.linkedTxNum, address, spentOutput.getValue()};
        txFile.write(blocksciInput);
        
        i++;
    }
    
    for (auto &output : tx->outputs) {
        auto [address, isNew] = resolveAddress(output.scriptOutput, addressState);
        if (isNew) {
            addressWriter.serialize(output.scriptOutput);
        }
        blocksci::Inout blocksciOutput{0, address, output.value};
        txFile.write(blocksciOutput);
    }
    return revealed;
};

template<typename InputQueue, typename OutputQueue, typename ProcessFunc, typename ShouldConsumeFunc>
void consumeTxes(boost::atomic<bool> &prevFinished, InputQueue &input, OutputQueue &output, ProcessFunc processFunc, ShouldConsumeFunc shouldConsumeFunc) {
    using namespace std::chrono_literals;
    auto consumeAll = [&]() {
        RawTransaction *rawTx = nullptr;
        while (input.pop(rawTx)) {
            processFunc(rawTx);
            if (shouldConsumeFunc(rawTx)) {
                assert(rawTx);
                while (!output.push(rawTx)) {
                    std::this_thread::sleep_for(200ms);
                }
            }
        }
    };
    
    while (!prevFinished) {
        consumeAll();
        std::this_thread::sleep_for(100ms);
    }
    
    consumeAll();
}

template<typename UpdateFunc>
class ProgressBar {
    uint64_t total;
    uint64_t percentageMarker;
    UpdateFunc updateFunc;
    
public:
    ProgressBar(uint64_t total_, UpdateFunc updateFunc_) : total(total_), updateFunc(updateFunc_) {
        auto percentage = static_cast<double>(total) / 1000.0;
        percentageMarker = static_cast<uint64_t>(std::ceil(percentage));
        std::cout.setf(std::ios::fixed,std::ios::floatfield);
        std::cout.precision(2);
    }
    
    ~ProgressBar() {
        std::cout << "\n";
    }
    
    template <typename... Args>
    void update(uint64_t currentCount, Args... args) {
        if (currentCount % 10000 == 0) {
            auto percentDone = (static_cast<double>(currentCount) / static_cast<double>(total)) * 100;
            std::cout << "\r" << percentDone << "% done";
            updateFunc(args...);
            std::cout << std::flush;;
        }
    }
};

template<class UpdateFunc>
ProgressBar<UpdateFunc> makeProgressBar(uint64_t total, UpdateFunc updateFunc) {
    return {total, updateFunc};
}

struct CompletionGuard {
    CompletionGuard(boost::atomic<bool> &isDone_) : isDone(isDone_) {}
    
    ~CompletionGuard() {
        isDone = true;
    }
private:
    boost::atomic<bool> &isDone;
};

NewBlocksFiles::NewBlocksFiles(const ParserConfigurationBase &config) : blockCoinbaseFile(config.blockCoinbaseFilePath()), blockFile(config.blockFilePath()), sequenceFile(config.sequenceFilePath()) {}

template <typename ParseTag>
std::vector<uint32_t> BlockProcessor::addNewBlocks(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> blocks, UTXOState &utxoState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile) {
    
    boost::atomic<bool> rawDone{false};
    boost::atomic<bool> hashDone{false};
    boost::atomic<bool> utxoDone{false};
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> hash_transaction_queue;
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> utxo_transaction_queue;
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<200000>> address_transaction_queue;
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> finished_transaction_queue;
    
    auto importer = std::async(std::launch::async, [&] {
        ProfilerRegisterThread();
        CompletionGuard guard(rawDone);
        auto loadFinishedTx = [&](RawTransaction *&tx) {
            return finished_transaction_queue.pop(tx);
        };
        
        auto outFunc = [&](RawTransaction *tx) {
            using namespace std::chrono_literals;
            while (!hash_transaction_queue.push(tx)) {
                std::this_thread::sleep_for(100ms);
            }
        };
        
        BlockFileReader<ParseTag> fileReader(config, blocks, currentTxNum);
        NewBlocksFiles files(config);
        
        for (auto &block : blocks) {
            fileReader.nextBlock(block, currentTxNum);
            readNewBlock(currentTxNum, block, fileReader, files, loadFinishedTx, outFunc);
            currentTxNum += block.nTx;
        }
        
        return fileReader;
    });
    
    auto hashCalculator = std::async(std::launch::async, [&] {
        ProfilerRegisterThread();
        CompletionGuard guard(hashDone);
        FixedSizeFileWriter<blocksci::uint256> hashFile{config.txHashesFilePath()};
        consumeTxes(rawDone, hash_transaction_queue, utxo_transaction_queue, [&](RawTransaction *tx) {
            calculateHash(tx, hashFile);
        }, [](RawTransaction *) { return true; });
    });
    
    auto utxoProcessor = std::async(std::launch::async, [&] {
        ProfilerRegisterThread();
        CompletionGuard guard(utxoDone);
        consumeTxes(hashDone, utxo_transaction_queue, address_transaction_queue, [&](RawTransaction *tx) {
            connectUTXOs(tx, utxoState);
        }, [](RawTransaction *) { return true; });
    });
    
    auto addressProcessor = std::async(std::launch::async, [&] {
        ProfilerRegisterThread();
        AddressWriter addressWriter{config};
        ECCVerifyHandle handle;
        
        std::vector<uint32_t> revealed;
        auto progressBar = makeProgressBar(totalTxCount, [=](RawTransaction *tx) {
            auto blockHeight = tx->blockHeight;
            std::cout << ", Block " << blockHeight << "/" << maxBlockHeight;
        });
        consumeTxes(utxoDone, address_transaction_queue, finished_transaction_queue, [&](RawTransaction *tx) {
            auto newRevealed = connectAddressess(tx, addressState, addressWriter, txFile);
            revealed.insert(revealed.end(), newRevealed.begin(), newRevealed.end());
            progressBar.update(tx->txNum - startingTxCount, tx);
            addressState.optionalSave();
        }, [&](RawTransaction *tx) {
            bool shouldSend = tx->sizeBytes < 800 && finished_transaction_queue.write_available() >= 1;
            if (!shouldSend) delete tx;
            return shouldSend;
        });
        return revealed;
    });
    
    auto reader = importer.get();
    hashCalculator.get();
    utxoProcessor.get();
    auto ret = addressProcessor.get();
    
    finished_transaction_queue.consume_all([](RawTransaction *tx) {
        delete tx;
    });
    
    return ret;
}


template <typename ParseTag>
std::vector<uint32_t> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> blocks, UTXOState &utxoState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile) {
    
    RawTransaction realTx;
    auto loadFinishedTx = [&](RawTransaction *&tx) {
        tx = &realTx;
        return true;
    };
        
    FixedSizeFileWriter<blocksci::uint256> hashFile{config.txHashesFilePath()};
    AddressWriter addressWriter{config};
    ECCVerifyHandle handle;
    
    std::vector<uint32_t> revealed;
    auto progressBar = makeProgressBar(totalTxCount, [=](RawTransaction *tx) {
        auto blockHeight = tx->blockHeight;
        std::cout << ", Block " << blockHeight << "/" << maxBlockHeight;
    });

    auto outFunc = [&](RawTransaction *tx) {
        calculateHash(tx, hashFile);
        connectUTXOs(tx, utxoState);
        auto newRevealed = connectAddressess(tx, addressState, addressWriter, txFile);
        revealed.insert(revealed.end(), newRevealed.begin(), newRevealed.end());
        progressBar.update(tx->txNum - startingTxCount, tx);
        addressState.optionalSave();
    };
        
    BlockFileReader<ParseTag> fileReader(config, blocks, currentTxNum);
    NewBlocksFiles files(config);
    
    for (auto &block : blocks) {
        fileReader.nextBlock(block, currentTxNum);
        readNewBlock(currentTxNum, block, fileReader, files, loadFinishedTx, outFunc);
        currentTxNum += block.nTx;
    }
    
    return revealed;
}

#ifdef BLOCKSCI_FILE_PARSER
template std::vector<uint32_t> BlockProcessor::addNewBlocks(const ParserConfiguration<FileTag> &config, std::vector<BlockInfo<FileTag>> nextBlocks, UTXOState &utxoState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
template std::vector<uint32_t> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<FileTag> &config, std::vector<BlockInfo<FileTag>> nextBlocks, UTXOState &utxoState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
#endif
#ifdef BLOCKSCI_RPC_PARSER
template std::vector<uint32_t> BlockProcessor::addNewBlocks(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> nextBlocks, UTXOState &utxoState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
template std::vector<uint32_t> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> nextBlocks, UTXOState &utxoState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
#endif
