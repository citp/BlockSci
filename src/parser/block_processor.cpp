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
#include "utxo_address_state.hpp"
#include "output_spend_data.hpp"

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
        uint32_t segwitMarker = *reinterpret_cast<const uint32_t *>(output.data.fullData.data());
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
        AnyScriptOutput scriptOutput(tx->outputs[static_cast<size_t>(i)].getScriptView(), tx->isSegwit);
        if (boost::apply_visitor(checker, scriptOutput.wrapped)) {
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
            tx->outputs.emplace_back(scriptBytes, 50 * 100000000.0);
            tx->hash = blocksci::uint256S("0100000000000000000000000000000000000000000000000000000000000000");
            tx->blockHeight = 0;
            tx->txNum = 0;
            tx->isSegwit = false;
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
            auto scriptView = tx->inputs[0].getScriptView();
            coinbase.assign(scriptView.begin(), scriptView.end());
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

void generateScriptOutputs(RawTransaction *tx) {
    tx->scriptOutputs.clear();
    for (auto &output : tx->outputs) {
        tx->scriptOutputs.emplace_back(output.getScriptView(), tx->isSegwit);
    }
}

void connectUTXOs(RawTransaction *tx, UTXOState &utxoState) {
    for (auto &input : tx->inputs) {
        auto utxo = utxoState.spendOutput(input.rawOutputPointer);
        input.type = utxo.type;
        input.linkedTxNum = utxo.txNum;
    }
    
    uint16_t i = 0;
    for (auto &scriptOutput : tx->scriptOutputs) {
        auto type = scriptOutput.type();
        if (isSpendable(scriptType(type))) {
            UTXO utxo{tx->txNum, type};
            RawOutputPointer pointer{tx->hash, i};
            utxoState.addOutput(utxo, pointer);
        }
        i++;
    }
}

void generateScriptInput(RawTransaction *tx, UTXOAddressState &utxoAddressState) {
    tx->scriptInputs.clear();
    uint16_t i = 0;
    for (auto &input : tx->inputs) {
        InputView inputView(i, tx->txNum, input.witnessStack, tx->isSegwit);
        auto spendData = utxoAddressState.spendOutput(input.getOutputPointer(), input.type);
        tx->scriptInputs.emplace_back(inputView, input.getScriptView(), *tx, spendData);
        i++;
    }
    
    i = 0;
    for (auto &scriptOutput : tx->scriptOutputs) {
        utxoAddressState.addOutput(scriptOutput, {tx->txNum, i});
        i++;
    }
}

void processAddresses(RawTransaction *tx, AddressState &addressState) {
    for (auto &scriptInput : tx->scriptInputs) {
        scriptInput.process(addressState);
    }
    
    for (auto &scriptOutput : tx->scriptOutputs) {
        scriptOutput.resolve(addressState);
    }
    
    addressState.optionalSave();
}

void serializeTransaction(RawTransaction *tx, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile) {
    
    blocksci::ArbitraryLengthData<blocksci::RawTransaction> data(tx->getRawTransaction());
    
    for (size_t i = 0; i < tx->inputs.size(); i++) {
        auto &input = tx->inputs[i];
        auto &scriptInput = tx->scriptInputs[i];
        auto spentTx = txFile.getData(input.linkedTxNum);
        auto &spentOutput = spentTx->getOutput(input.rawOutputPointer.outputNum);
        auto address = spentOutput.getAddress();
        scriptInput.setScriptNum(address.scriptNum);
        
        spentOutput.linkedTxNum = tx->txNum;
        blocksci::Inout blocksciInput{input.linkedTxNum, address, spentOutput.getValue()};
        data.add(blocksciInput);
    }
    
    for (size_t i = 0; i < tx->outputs.size(); i++) {
        auto &output = tx->outputs[i];
        auto &scriptOutput = tx->scriptOutputs[i];
        blocksci::Inout blocksciOutput{0, scriptOutput.address(), output.value};
        data.add(blocksciOutput);
    }
    
    txFile.write(data);
}

std::vector<blocksci::Script> serializeAddressess(RawTransaction *tx, AddressWriter &addressWriter) {
    std::vector<blocksci::Script> revealed;
    
    for (auto &scriptInput : tx->scriptInputs) {
        ProcessedInput processedInput = addressWriter.serialize(scriptInput);
        revealed.insert(revealed.end(), processedInput.begin(), processedInput.end());
    }
    
    for (auto &scriptOutput : tx->scriptOutputs) {
        addressWriter.serialize(scriptOutput);
    }
    
    return revealed;
}

struct CompletionGuard {
    CompletionGuard(boost::atomic<bool> &isDone_) : isDone(isDone_) {}
    
    ~CompletionGuard() {
        isDone = true;
    }
private:
    boost::atomic<bool> &isDone;
};

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

struct NextQueueFinishedEarlyException : public std::runtime_error {
    NextQueueFinishedEarlyException() : std::runtime_error("Next queue finished early") {}
    virtual ~NextQueueFinishedEarlyException() = default;
};

template <typename ProcessFunc, typename AdvanceFunc>
class ProcessStep {
public:
    boost::atomic<bool> &prevDone;
    
    boost::atomic<bool> isDone{false};
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<10000>> inputQueue;
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<10000>> *nextQueue;
    boost::atomic<bool> *nextDone;
    
    ProcessFunc func;
    AdvanceFunc advanceFunc;
    
    long prevWaitCount = 0;
    long nextWaitCount = 0;
    
    // AdvanceFunc
    ProcessStep(boost::atomic<bool> &prevDone_, ProcessFunc func_, AdvanceFunc advanceFunc_) : prevDone(prevDone_), func(func_), advanceFunc(advanceFunc_) {}
    
    template <typename PrevStep>
    ProcessStep(PrevStep &prevStep, ProcessFunc func_, AdvanceFunc advanceFunc_) : ProcessStep(prevStep.isDone, func_, advanceFunc_) {
        prevStep.nextQueue = &inputQueue;
        prevStep.nextDone = &isDone;
    }
    
    void operator()() {
        using namespace std::chrono_literals;
        CompletionGuard guard(isDone);
        auto consumeAll = [&]() {
            RawTransaction *rawTx = nullptr;
            while (inputQueue.pop(rawTx)) {
                func(rawTx);
                if (advanceFunc(rawTx)) {
                    assert(rawTx);
                    while (!nextQueue->push(rawTx)) {
                        if (nextDone && *nextDone) {
                            throw NextQueueFinishedEarlyException();
                        }
                        nextWaitCount++;
                        std::this_thread::sleep_for(5ms);
                    }
                }
            }
        };
        
        while (!prevDone) {
            consumeAll();
            prevWaitCount++;
            std::this_thread::sleep_for(5ms);
        }
        
        consumeAll();
    }
};

template<typename ProcessFunc, typename AdvanceFunc, typename PrevStep>
ProcessStep<ProcessFunc, AdvanceFunc> makeProcessStep(PrevStep &prevStep, ProcessFunc func, AdvanceFunc advanceFunc) {
    return ProcessStep<ProcessFunc, AdvanceFunc>(prevStep, func, advanceFunc);
}

template<typename ProcessFunc, typename AdvanceFunc>
ProcessStep<ProcessFunc, AdvanceFunc> makeProcessStep(boost::atomic<bool> &prevDone, ProcessFunc func, AdvanceFunc advanceFunc) {
    return ProcessStep<ProcessFunc, AdvanceFunc>(prevDone, func, advanceFunc);
}

NewBlocksFiles::NewBlocksFiles(const ParserConfigurationBase &config) : blockCoinbaseFile(config.blockCoinbaseFilePath()), blockFile(config.blockFilePath()), sequenceFile(config.sequenceFilePath()) {}

template <typename ParseTag>
std::vector<blocksci::Script> BlockProcessor::addNewBlocks(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> blocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile) {
    
    boost::atomic<bool> rawDone{false};
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<10000>> finished_transaction_queue;
    
    FixedSizeFileWriter<blocksci::uint256> hashFile{config.txHashesFilePath()};
    AddressWriter addressWriter{config};
    
    std::vector<blocksci::Script> revealed;
    auto progressBar = makeProgressBar(totalTxCount, [=](RawTransaction *tx) {
        auto blockHeight = tx->blockHeight;
        std::cout << ", Block " << blockHeight << "/" << maxBlockHeight;
    });
    
    auto advanceFunc = [](RawTransaction *) { return true; };
    
    auto calculateHashesStep = makeProcessStep(rawDone, [&](RawTransaction *tx) {
        calculateHash(tx, hashFile);
    }, advanceFunc);
    
    
    auto generateScriptOutputsStep = makeProcessStep(calculateHashesStep, generateScriptOutputs, advanceFunc);
    
    auto connectUTXOsStep = makeProcessStep(generateScriptOutputsStep, [&](RawTransaction *tx) {
        connectUTXOs(tx, utxoState);
    }, advanceFunc);
    
    auto generateScriptInputStep = makeProcessStep(connectUTXOsStep, [&](RawTransaction *tx) {
        generateScriptInput(tx, utxoAddressState);
    }, advanceFunc);
    
    auto processAddressStep = makeProcessStep(generateScriptInputStep, [&](RawTransaction *tx) {
        processAddresses(tx, addressState);
    }, advanceFunc);
    
    auto serializeTransactionStep = makeProcessStep(processAddressStep, [&](RawTransaction *tx) {
        serializeTransaction(tx, txFile);
    }, advanceFunc);
    
    auto serializeAddressStep = makeProcessStep(serializeTransactionStep, [&](RawTransaction *tx) {
        auto newRevealed = serializeAddressess(tx, addressWriter);
        revealed.insert(revealed.end(), newRevealed.begin(), newRevealed.end());
        progressBar.update(tx->txNum - startingTxCount, tx);
    }, [&](RawTransaction *tx) {
        bool shouldSend = tx->sizeBytes < 800 && finished_transaction_queue.write_available() >= 1;
        if (!shouldSend) delete tx;
        return shouldSend;
    });
    serializeAddressStep.nextQueue = &finished_transaction_queue;
    
    long nextWaitCount = 0;
    
    auto importer = std::async(std::launch::async, [&] {
        CompletionGuard guard(rawDone);
        auto loadFinishedTx = [&](RawTransaction *&tx) {
            return finished_transaction_queue.pop(tx);
        };
        
        auto outFunc = [&](RawTransaction *tx) {
            using namespace std::chrono_literals;
            while (!calculateHashesStep.inputQueue.push(tx)) {
                if (calculateHashesStep.isDone) {
                    throw NextQueueFinishedEarlyException();
                }
                nextWaitCount++;
                std::this_thread::sleep_for(5ms);
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
    
    auto calculateHashesStepFuture = std::async(std::launch::async, [&]() {
        calculateHashesStep();
    });
    
    auto generateScriptOutputsStepFuture = std::async(std::launch::async, [&]() {
        generateScriptOutputsStep();
    });
    
    auto connectUTXOsStepFuture = std::async(std::launch::async, [&]() {
        connectUTXOsStep();
    });
    
    auto generateScriptInputStepFuture = std::async(std::launch::async, [&] {
        ECCVerifyHandle handle;
        generateScriptInputStep();
    });
    
    auto processAddressStepFuture = std::async(std::launch::async, [&] {
        ECCVerifyHandle handle;
        processAddressStep();
    });
    
    auto serializeTransactionStepFuture = std::async(std::launch::async, [&] {
        serializeTransactionStep();
    });
    
    auto serializeAddressStepFuture = std::async(std::launch::async, [&] {
        serializeAddressStep();
    });
    
    auto reader = importer.get();
    calculateHashesStepFuture.get();
    generateScriptOutputsStepFuture.get();
    connectUTXOsStepFuture.get();
    generateScriptInputStepFuture.get();
    processAddressStepFuture.get();
    serializeTransactionStepFuture.get();
    serializeAddressStepFuture.get();
    
    std::cout << "generateTxesStep:" << nextWaitCount << "\n";
    std::cout << "calculateHashesStep: " << calculateHashesStep.prevWaitCount << " " << calculateHashesStep.nextWaitCount << "\n";
    std::cout << "generateScriptOutputsStep: " << generateScriptOutputsStep.prevWaitCount << " " << generateScriptOutputsStep.nextWaitCount << "\n";
    std::cout << "connectUTXOsStep: " << connectUTXOsStep.prevWaitCount << " " << connectUTXOsStep.nextWaitCount << "\n";
    std::cout << "generateScriptInputStep: " << generateScriptInputStep.prevWaitCount << " " << generateScriptInputStep.nextWaitCount << "\n";
    std::cout << "processAddressStep: " << processAddressStep.prevWaitCount << " " << processAddressStep.nextWaitCount << "\n";
    std::cout << "serializeTransactionStep: " << serializeTransactionStep.prevWaitCount << " " << serializeTransactionStep.nextWaitCount << "\n";
    std::cout << "serializeAddressStep: " << serializeAddressStep.prevWaitCount << " " << serializeAddressStep.nextWaitCount << "\n";
    
    finished_transaction_queue.consume_all([](RawTransaction *tx) {
        delete tx;
    });
    
    return revealed;
}


template <typename ParseTag>
std::vector<blocksci::Script> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> blocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile) {
    
    RawTransaction realTx;
    auto loadFinishedTx = [&](RawTransaction *&tx) {
        tx = &realTx;
        return true;
    };
        
    FixedSizeFileWriter<blocksci::uint256> hashFile{config.txHashesFilePath()};
    AddressWriter addressWriter{config};
    ECCVerifyHandle handle;
    
    std::vector<blocksci::Script> revealed;
    auto progressBar = makeProgressBar(totalTxCount, [=](RawTransaction *tx) {
        auto blockHeight = tx->blockHeight;
        std::cout << ", Block " << blockHeight << "/" << maxBlockHeight;
    });

    auto outFunc = [&](RawTransaction *tx) {
        calculateHash(tx, hashFile);
        connectUTXOs(tx, utxoState);
        generateScriptInput(tx, utxoAddressState);
        processAddresses(tx, addressState);
        serializeTransaction(tx, txFile);
        auto newRevealed = serializeAddressess(tx, addressWriter);
        revealed.insert(revealed.end(), newRevealed.begin(), newRevealed.end());
        progressBar.update(tx->txNum - startingTxCount, tx);
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
template std::vector<blocksci::Script> BlockProcessor::addNewBlocks(const ParserConfiguration<FileTag> &config, std::vector<BlockInfo<FileTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
template std::vector<blocksci::Script> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<FileTag> &config, std::vector<BlockInfo<FileTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
#endif
#ifdef BLOCKSCI_RPC_PARSER
template std::vector<blocksci::Script> BlockProcessor::addNewBlocks(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
template std::vector<blocksci::Script> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);
#endif
