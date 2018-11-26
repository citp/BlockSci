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
#include "address_writer.hpp"
#include "safe_mem_reader.hpp"
#include "chain_index.hpp"
#include "preproccessed_block.hpp"
#include "chain_index.hpp"
#include "utxo_state.hpp"
#include "address_state.hpp"
#include "utxo_address_state.hpp"
#include "output_spend_data.hpp"
#include "serializable_map.hpp"
#include "file_writer.hpp"

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/bitcoinapi.h>
#endif

#include <boost/lockfree/spsc_queue.hpp>

#include <atomic>
#include <cmath>
#include <fstream>
#include <future>
#include <iostream>
#include <thread>
#include <list>

std::vector<unsigned char> ParseHex(const char* psz);

BlockProcessor::BlockProcessor(uint32_t startingTxCount_, uint64_t startingInputCount, uint64_t startingOutputCount, uint32_t totalTxCount_, blocksci::BlockHeight maxBlockHeight_) : startingTxCount(startingTxCount_), currentTxNum(startingTxCount_), currentInputNum(startingInputCount), currentOutputNum(startingOutputCount), totalTxCount(totalTxCount_), maxBlockHeight(maxBlockHeight_) {
    
}

std::vector<unsigned char> ParseHex(const char* psz) {
    // convert hex dump to vector
    std::vector<unsigned char> vch;
    while (true) {
        while (isspace(*psz)) {
            psz++;
        }
        signed char c = blocksci::HexDigit(*psz++);
        if (c == static_cast<signed char>(-1)) {
            break;
        }
        auto n = static_cast<unsigned char>(c << 4);
        c = blocksci::HexDigit(*psz++);
        if (c == static_cast<signed char>(-1)) {
            break;
        }
        n |= c;
        vch.push_back(n);
    }
    return vch;
}

BlockFileReaderBase::~BlockFileReaderBase() = default;

template <typename ParseTag>
class BlockFileReader;

#ifdef BLOCKSCI_FILE_PARSER

template <>
class BlockFileReader<FileTag> : public BlockFileReaderBase {
    // Map of (blkXXXXX.dat file number) -> pair(SafeMemReader for blkXXXXX.dat file, last tx number of this blkXXXXX.dat file)
    std::unordered_map<int, std::pair<SafeMemReader, uint32_t>> files;

    // Map of (blkXXXXX.dat file number) -> (last tx number of this blkXXXXX.dat file)
    std::unordered_map<int, uint32_t> lastTxRequired;

    const ParserConfiguration<FileTag> &config;
    SafeMemReader *reader = nullptr;
    
    blocksci::BlockHeight currentHeight = 0;
    uint32_t currentTxNum = 0;

    template<bool shouldAdvance>
    void nextTxImp(RawTransaction *tx, bool isSegwit) {
        try {
            auto firstTxOffset = reader->offset();
            tx->load(*reader, currentTxNum, currentHeight, isSegwit);
            if (shouldAdvance) {
                currentTxNum++;
            } else {
                reader->reset(firstTxOffset);
            }
        } catch (const std::exception &e) {
            std::cerr << "Failed to load tx"
            << " from block" << currentHeight
            << " at offset " << reader->offset()
            << ".\n" << e.what();
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
            if (!blockPath.exists()) {
                std::stringstream ss;
                ss << "Error: Failed to open block file " << blockPath << "\n";
                throw std::runtime_error(ss.str());
            }
            files.insert(std::make_pair(block.nFile, std::make_pair(SafeMemReader(blockPath.str()), lastTxRequired[block.nFile])));
        }
        reader = &files.at(block.nFile).first;
        reader->reset(block.nDataPos);
        reader->advance(sizeof(CBlockHeader));
        reader->readVariableLengthInteger();
        currentHeight = block.height;
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
    
    uint32_t firstTxNum = 0;
    uint32_t currentTxOffset = 0;
    blocksci::BlockHeight currentHeight = 0;
    BlockInfo<RPCTag> block;
    
    template<bool shouldAdvance>
    void nextTxImp(RawTransaction *tx, bool isSegwit) {
        if (currentHeight == 0) {
            tx->outputs.clear();
            tx->outputs.reserve(1);
            
            auto scriptPubKey = blocksci::CScript() << ParseHex("040184a11fa689ad5123690c81a3a49c8f13f8d45bac857fbcbc8bc4a8ead3eb4b1ff4d4614fa18dce611aaf1f471216fe1b51851b4acf21b17fc45171ac7b13af") << blocksci::OP_CHECKSIG;
            std::vector<unsigned char> scriptBytes(scriptPubKey.begin(), scriptPubKey.end());
            //Set the desired initial block reward
            tx->outputs.emplace_back(scriptBytes, 50l * 100000000l);
            tx->hash = blocksci::uint256S("0100000000000000000000000000000000000000000000000000000000000000");
            tx->blockHeight = blocksci::BlockHeight{0};
            tx->txNum = 0;
            tx->isSegwit = false;
        } else {
            auto txinfo = bapi.getrawtransaction(block.tx[currentTxOffset], 1);
            tx->load(txinfo, firstTxNum + currentTxOffset, currentHeight, isSegwit);
        }
        if (shouldAdvance) {
            currentTxOffset++;
        }
    }
    
public:
    BlockFileReader(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> &, uint32_t) : bapi(config.createBitcoinAPI()) {}
    
    void nextBlock(BlockInfo<RPCTag> &block_, uint32_t txNum) {
        block = block_;
        currentHeight = block.height;
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

// Process single block and its transactions
blocksci::RawBlock readNewBlock(uint32_t firstTxNum, uint64_t firstInputNum, uint64_t firstOutputNum, const BlockInfoBase &block, BlockFileReaderBase &fileReader, NewBlocksFiles &files, const std::function<bool(RawTransaction *&tx)> &loadFunc, const std::function<void(RawTransaction *tx)> &outFunc, bool isSegwit) {
    std::vector<unsigned char> coinbase;
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    uint32_t headerSize = 80 + variableLengthIntSize(block.nTx);
    uint32_t baseSize = headerSize;
    uint32_t realSize = headerSize;
    uint32_t inputCount = 0;
    uint32_t outputCount = 0;

    // Iterate over all transactions of the current block
    for (uint32_t j = 0; j < block.nTx; j++) {
        RawTransaction *tx = nullptr;

        // Try to re-use memory from transactions that have passed the entire queue
        if (!loadFunc(tx)) {
            // No tx is currently available in finished_transaction_queue -> allocate new memory
            tx = new RawTransaction();
        } else {
            // loadFunc(tx) set the pointer tx to memory from a finished RawTransaction -> reuse that space
            assert(tx);
            fileReader.receivedFinishedTx(tx);
        }

        // Read next transaction into tx
        fileReader.nextTx(tx, isSegwit);

        // For every tx, write blockchain-wide number of the first tx input and ouput to file (FixedSizeFileMapper<uint64_t>)
        files.txFirstInput.write(firstInputNum + inputCount);
        files.txFirstOutput.write(firstOutputNum + outputCount);

        // Write tx version to file (FixedSizeFileMapper<int32_t>)
        files.txVersionFile.write(tx->version);

        // If transaction is a coinbase transaction, extract and assign coinbase data
        if (tx->inputs.size() == 1 && tx->inputs[0].rawOutputPointer.hash == nullHash) {
            auto scriptView = tx->inputs[0].getScriptView();
            coinbase.assign(scriptView.begin(), scriptView.end());
            tx->inputs.clear();  // do not store inputs for coinbase transactions
        }

        for (auto &input : tx->inputs) {
            // Write blockchain's sequence number field to file (FixedSizeFileMapper<uint32_t>)
            files.inputSequenceFile.write(input.sequenceNum);

            // Write the tx-internal output number of the spent output to file (FixedSizeFileMapper<uint16_t>)
            files.inputSpentOutNumFile.write(input.rawOutputPointer.outputNum);
        }
        
        inputCount += tx->inputs.size();
        outputCount += tx->outputs.size();
        baseSize += tx->baseSize;
        realSize += tx->realSize;

        // add tx to the queue of the first step of the processing pipeline
        outFunc(tx);
    }

    // Instantiate RawBlock object
    blocksci::RawBlock blocksciBlock{firstTxNum, block.nTx, inputCount, outputCount, static_cast<uint32_t>(static_cast<int>(block.height)), block.hash, block.header.nVersion, block.header.nTime, block.header.nBits, block.header.nNonce, realSize, baseSize, files.blockCoinbaseFile.size()};
    files.blockCoinbaseFile.write(coinbase.begin(), coinbase.end());
    return blocksciBlock;
}


// Definition of all functions for the processing pipeline

// 1. Step: Calculate hash of transaction and write it to the hash file (chain/tx_hashes.dat)
void CalculateHashStep::processOutputs(RawTransaction &tx) {
    tx.calculateHash();
    hashFile.write(tx.hash);
}

void CalculateHashStep::processInputs(RawTransaction &) {}

// 2. Step: Parse the output scripts (into CScriptView) of the transaction in order to identify address types and extract relevant information.

void GenerateScriptOutputsStep::processOutputs(RawTransaction &tx) {
    tx.scriptOutputs.clear();
    tx.scriptOutputs.reserve(tx.outputs.size());
    for (auto &output : tx.outputs) {
        // 2nd param assumes p2sh is always active
        // TODO: Add flag to disable p2sh
        tx.scriptOutputs.emplace_back(output.getScriptView(), true, tx.isSegwit);
    }
}

void GenerateScriptOutputsStep::processInputs(RawTransaction &) {}

// 3. Step: Store information about the spent output with each input of the transaction. Then store information about each output for future lookup.

void ConnectUTXOsStep::processOutputs(RawTransaction &tx) {
    // Fill UTXOState (SerializableMap<RawOutputPointer, UTXO>) with mapping tx output ->  UTXO(output.value, txNum, type)
    for (uint16_t i = 0; i < tx.outputs.size(); i++) {
        auto &output = tx.outputs[i];
        auto &scriptOutput = tx.scriptOutputs[i];
        auto type = scriptOutput.type();
        if (isSpendable(dedupType(type))) {
            UTXO utxo{output.value, tx.txNum, type};
            RawOutputPointer pointer{tx.hash, i};
            utxoState.add(pointer, utxo);
        }
    }
}

void ConnectUTXOsStep::processInputs(RawTransaction &tx) {
    for (auto &input : tx.inputs) {
        // remove the output that this input spends from the UTXOState and assign it to the input
        input.utxo = utxoState.erase(input.rawOutputPointer);
    }
}

/* 4. Step: Parse the input script of each input based information about the associated output script.
 *    Then store information about each output address for future lookup. */

void GenerateScriptInputStep::processOutputs(RawTransaction &tx) {
    uint16_t i = 0;
    for (auto &scriptOutput : tx.scriptOutputs) {
        utxoAddressState.addOutput(AnySpendData{scriptOutput}, {tx.txNum, i});
        i++;
    }
}

void GenerateScriptInputStep::processInputs(RawTransaction &tx) {
    tx.scriptInputs.clear();
    tx.scriptInputs.reserve(tx.inputs.size());
    uint16_t i = 0;
    for (auto &input : tx.inputs) {
        InputView inputView(i, tx.txNum, input.getWitnessStack(), tx.isSegwit);
        auto spendData = utxoAddressState.spendOutput(input.getOutputPointer(), input.utxo.type);
        tx.scriptInputs.emplace_back(inputView, input.getScriptView(), tx, spendData);
        i++;
    }
}

/* 5. Step: Attach a scriptNum to each script in the transaction. For address types which are
      deduplicated (Pubkey, ScriptHash, Multisig and their varients) use the previously allocated
      scriptNum if the address was seen before. Increment the scriptNum counter for newly seen addresses. */

void ProcessAddressesStep::processOutputs(RawTransaction &tx) {
    for (auto &scriptOutput : tx.scriptOutputs) {
        scriptOutput.resolve(addressState);
    }
}

void ProcessAddressesStep::processInputs(RawTransaction &tx) {
    for (auto &scriptInput : tx.scriptInputs) {
        scriptInput.process(addressState);
    }
}

/* 6. Step: Record the scriptNum for each output for later reference. Assign each spent input with
            the scriptNum of the output its spending */

void RecordAddressesStep::processOutputs(RawTransaction &tx) {
    uint16_t i = 0;
    for (auto &scriptOutput : tx.scriptOutputs) {
        auto scriptNum = scriptOutput.address().scriptNum;
        assert(scriptNum > 0);
        state.add({tx.txNum, i}, scriptNum);
        i++;
    }
}

void RecordAddressesStep::processInputs(RawTransaction &tx) {
    for (size_t i = 0; i < tx.inputs.size(); i++) {
        auto &input = tx.inputs[i];
        auto &scriptInput = tx.scriptInputs[i];
        auto scriptNum = state.erase(input.getOutputPointer());
        assert(scriptNum > 0);
        scriptInput.setScriptNum(scriptNum);
    }
}

// 7. Step: Serialize transaction data, inputs, and outputs and write them to the txFile

void SerializeTransactionStep::processOutputs(RawTransaction &) {
    
}

void SerializeTransactionStep::processInputs(RawTransaction &tx) {
    txFile.writeIndexGroup();
    txFile.write(tx.getRawTransaction());
    
    for (size_t i = 0; i < tx.inputs.size(); i++) {
        auto &input = tx.inputs[i];
        auto &scriptInput = tx.scriptInputs[i];
        linkDataFile.write({input.getOutputPointer(), tx.txNum});
        auto address = scriptInput.address();
        blocksci::Inout blocksciInput{input.utxo.txNum, address.scriptNum, address.type, input.utxo.value};
        txFile.write(blocksciInput);
    }
    
    for (size_t i = 0; i < tx.outputs.size(); i++) {
        auto &output = tx.outputs[i];
        auto &scriptOutput = tx.scriptOutputs[i];
        auto address = scriptOutput.address();
        blocksci::Inout blocksciOutput{0, address.scriptNum, address.type, output.value};
        txFile.write(blocksciOutput);
    }
    std::cout << "Serialized tx " << tx.txNum << "with index file count " << txFile.size() << "\n";
    if (tx.txNum == 347) {
        std::cout << "Serialized tx " << tx.txNum << "\n";
    }
}

// 8. Step: Save address data into files for the analysis library

void SerializeAddressesStep::processOutputs(RawTransaction &tx) {
    for (auto &scriptOutput : tx.scriptOutputs) {
        addressWriter.serialize(scriptOutput, tx.txNum, true);
    }
}

void SerializeAddressesStep::processInputs(RawTransaction &tx) {
    for (size_t i = 0; i < tx.inputs.size(); i++) {
        auto &input = tx.inputs[i];
        auto &scriptInput = tx.scriptInputs[i];
        addressWriter.serialize(scriptInput, tx.txNum, input.utxo.txNum);
    }
}


void backUpdateTxes(const ParserConfigurationBase &config) {
    std::vector<OutputLinkData> updates;
    
    std::cout << "Back linking transactions" << std::endl;
    
    {
        blocksci::FixedSizeFileMapper<OutputLinkData> linkDataFile(config.txUpdatesFilePath());
        updates.reserve(static_cast<size_t>(linkDataFile.size()));
        
        for (uint32_t i = 0; i < linkDataFile.size(); i++) {
            updates.push_back(*linkDataFile[i]);
        }
        
        std::sort(updates.begin(), updates.end(), [](const auto& a, const auto& b) {
            return a.pointer < b.pointer;
        });
    }
    
    {
        blocksci::IndexedFileMapper<mio::access_mode::write, blocksci::RawTransaction> txFile(blocksci::ChainAccess::txFilePath(config.dataConfig.chainDirectory()));
        auto progressBar = blocksci::makeProgressBar(updates.size(), [=]() {});
        
        uint32_t count = 0;
        for (auto &update : updates) {
            auto tx = txFile.getData(update.pointer.txNum);
            auto &output = tx->getOutput(update.pointer.inoutNum);
            // Set the forward-reference to the tx number of the tx that contains the spending input
            output.setLinkedTxNum(update.txNum);
            
            count++;
            progressBar.update(count);
        }
    }
    filesystem::path{config.txUpdatesFilePath() + ".dat"}.remove_file();
}

struct CompletionGuard {
    explicit CompletionGuard(std::atomic<bool> &isDone_) : isDone(isDone_) {}
    CompletionGuard(const CompletionGuard &) = delete;
    CompletionGuard &operator=(const CompletionGuard &) = delete;
    CompletionGuard(CompletionGuard &&) = delete;
    CompletionGuard &operator=(CompletionGuard &&) = delete;
    ~CompletionGuard() {
        isDone = true;
    }
private:
    std::atomic<bool> &isDone;
};

struct NextQueueFinishedEarlyException : public std::runtime_error {
    NextQueueFinishedEarlyException() : std::runtime_error("Next queue finished early") {}
};

using TxQueue = boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<10000>>;
using AdvanceFunc = std::function<bool(RawTransaction *, TxQueue *)>;

template <typename Func>
bool processNext(TxQueue &inputQueue, TxQueue *nextQueue, std::atomic<bool> *nextDone, std::function<bool(RawTransaction *, TxQueue *)> advanceFunc, Func && processFunc, int64_t &nextWaitCount) {
    using namespace std::chrono_literals;
    
    RawTransaction *rawTx = nullptr;
    inputQueue.pop(rawTx);
    if (rawTx) {
        // Execute processing step on rawTx, eg. calculateHashesFunc or connectUTXOsFunc
        std::forward<Func>(processFunc)(rawTx);
        // Check if advanceFunc is successful before further processing the pipeline
        if (advanceFunc(rawTx, nextQueue)) {
            assert(rawTx);
            // Add rawTx to the next queue. if it fails (queue is full), wait 5ms and try again
            while (!nextQueue->push(rawTx)) {
                if (nextDone && *nextDone) {
                    // Error: next ProcessStep finished before all items were queued
                    throw NextQueueFinishedEarlyException();
                }
                nextWaitCount++;
                std::this_thread::sleep_for(5ms);
            }
        }
        return true;
    } else {
        return false;
    }
};

class ProcessStep {
public:
    std::atomic<bool> &prevDone;
    std::atomic<bool> *prevDoneSecond = nullptr;
    
    std::atomic<bool> isDoneFirst{false};
    std::atomic<bool> isDoneSecond{false};

    /* Single-producer/single-consumer fifo queues, pushing and popping is wait-free
     *
     * size of the ringbuffer is specified by boost::lockfree::capacity<>
     */
    TxQueue inputQueueFirst;
    TxQueue inputQueueSecond;
    TxQueue *nextQueueFirst = nullptr;
    TxQueue *nextQueueSecond = nullptr;
    
    std::atomic<bool> *nextDoneFirst = nullptr;
    std::atomic<bool> *nextDoneSecond = nullptr;
    
    std::unique_ptr<ProcessorStep> func;
    AdvanceFunc advanceFuncFirst;
    AdvanceFunc advanceFuncSecond;
    
    int64_t prevWaitCount = 0;
    int64_t nextWaitCount = 0;
    
    // AdvanceFunc
    ProcessStep(std::atomic<bool> &prevDone_, std::unique_ptr<ProcessorStep> && func_, const AdvanceFunc &advanceFuncFirst_, const AdvanceFunc &advanceFuncSecond_) : prevDone(prevDone_), func(std::move(func_)), advanceFuncFirst(advanceFuncFirst_), advanceFuncSecond(advanceFuncSecond_) {}
    
    template <typename PrevStep>
    ProcessStep(PrevStep &prevStep, std::unique_ptr<ProcessorStep> && func_, const AdvanceFunc &advanceFuncFirst_, const AdvanceFunc &advanceFuncSecond_) : ProcessStep(prevStep.isDoneFirst, std::move(func_), advanceFuncFirst_, advanceFuncSecond_) {
        // Link the queue- and done-pointers for the previous queue to this object's variables
        prevDoneSecond = &prevStep.isDoneSecond;
        prevStep.nextQueueFirst = &inputQueueFirst;
        prevStep.nextQueueSecond = &inputQueueSecond;
        prevStep.nextDoneFirst = &isDoneFirst;
        prevStep.nextDoneSecond = &isDoneSecond;
    }
    // inputProcessingDone
    void operator()() {
        using namespace std::chrono_literals;
        // CompletionGuard sets isDone to true in its destructor that is called at the end of this operator() method
        CompletionGuard firstGuard(isDoneFirst);
        CompletionGuard secondGuard(isDoneSecond);
        
        auto consumeAll = [&]() {
            bool processedSecond = true;
            bool processedFirst = true;
            while (processedSecond || processedFirst) {
                processedSecond = processNext(inputQueueSecond, nextQueueSecond, nextDoneSecond, advanceFuncSecond, [&](RawTransaction *tx) {
                    func->processInputs(*tx);
                }, nextWaitCount);
                
                processedFirst = processNext(inputQueueFirst, nextQueueFirst, nextDoneFirst, advanceFuncFirst, [&](RawTransaction *tx) {
                    func->processOutputs(*tx);
                }, nextWaitCount);
            }
        };

        // Consume queued items as long as the previous processing step has not finished
        assert (prevDoneSecond != nullptr);
        while (!prevDone || !*prevDoneSecond) {
            consumeAll();
            prevWaitCount++;
            std::this_thread::sleep_for(5ms);
            
            if (prevDone) {
                consumeAll();
                isDoneFirst = true;
            }
        }

        // Last call to consume queued items to catch last items
        consumeAll();
    }
};

struct ProcessStepQueue {
    
    std::atomic<bool> importDone{false};
    
    // Queue for RawTransaction objects that have gone through the entire processing pipeline
    TxQueue finishedQueue;
    
    std::list<ProcessStep> steps;
    std::vector<std::future<void>> futures;
    
    ProcessStepQueue(std::unique_ptr<ProcessorStep> && func, const AdvanceFunc &advanceFuncFirst, const AdvanceFunc &advanceFuncSecond) {
        steps.emplace_back(importDone, std::move(func), advanceFuncFirst, advanceFuncSecond);
    }
    
    void addStep(std::unique_ptr<ProcessorStep> && func, const AdvanceFunc &advanceFuncFirst, const AdvanceFunc &advanceFuncSecond) {
        steps.emplace_back(steps.back(), std::move(func), advanceFuncFirst, advanceFuncSecond);
    }
    
    TxQueue &inputQueue() {
        return steps.front().inputQueueFirst;
    }
    
    bool isRunning() {
        return steps.front().isDoneFirst;
    }
    
    void run() {
        // Add finished_transaction_queue as the last queue after the last actual processing step
        steps.back().nextQueueFirst = &steps.front().inputQueueSecond;
        steps.back().nextQueueSecond = &finishedQueue;
        
        steps.front().prevDoneSecond = &steps.back().isDoneFirst;
        
        for (auto &step : steps) {
            futures.push_back(std::async(std::launch::async, [&]() {
                step();
            }));
        }
    }
    
    void waitForComplete() {
        for (auto &future : futures) {
            future.get();
        }
        
        // free all RawTransaction memory slots
        finishedQueue.consume_all([](RawTransaction *tx) {
            delete tx;
        });
    }
};

NewBlocksFiles::NewBlocksFiles(const ParserConfigurationBase &config) :
    blockCoinbaseFile(blocksci::ChainAccess::blockCoinbaseFilePath(config.dataConfig.chainDirectory())),
    txFirstInput(blocksci::ChainAccess::firstInputFilePath(config.dataConfig.chainDirectory())),
    txFirstOutput(blocksci::ChainAccess::firstOutputFilePath(config.dataConfig.chainDirectory())),
    txVersionFile(blocksci::ChainAccess::txVersionFilePath((config.dataConfig.chainDirectory()))),
    inputSpentOutNumFile(blocksci::ChainAccess::inputSpentOutNumFilePath(config.dataConfig.chainDirectory())),
    inputSequenceFile(blocksci::ChainAccess::sequenceFilePath(config.dataConfig.chainDirectory())) {}

template <typename ParseTag>
std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocks(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> blocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState) {

    AddressWriter addressWriter{config};
    IndexedFileWriter<1> txFile(blocksci::ChainAccess::txFilePath(config.dataConfig.chainDirectory()));
    FixedSizeFileWriter<OutputLinkData> linkDataFile(config.txUpdatesFilePath());
    FixedSizeFileWriter<blocksci::uint256> txHashFile{blocksci::ChainAccess::txHashesFilePath(config.dataConfig.chainDirectory())};

    auto advanceFunc = [](RawTransaction *, TxQueue *) { return true; };
    
    auto progressBar = blocksci::makeProgressBar(totalTxCount, [=](RawTransaction *tx) {
        auto blockHeight = tx->blockHeight;
        std::cout << ", Block " << blockHeight << "/" << maxBlockHeight;
    });
    
    /* Advance function of the last step
     * Optimization: Only push tx to finished_transaction_queue if it meets certain conditions, otherwise de-allocate it */
    
    auto serializeAddressAdvanceFunc = [&](RawTransaction *tx,  TxQueue *nextQueue) {
        progressBar.update(tx->txNum - startingTxCount, tx);
        bool shouldSend = tx->realSize < 800 && nextQueue->write_available() >= 1;
        if (!shouldSend)  {
            delete tx;
        }
        return shouldSend;
    };
    
    // Definition of all ProcessStep objects for the processing pipeline
    
    // 1. Step: Calculate hash of transaction and write it to the hash file (chain/tx_hashes.dat)
    ProcessStepQueue processQueue(std::make_unique<CalculateHashStep>(txHashFile), advanceFunc, advanceFunc);

    // 2. Step: Parse the output scripts (into CScriptView) of the transaction in order to identify address types and extract relevant information.
    processQueue.addStep(std::make_unique<GenerateScriptOutputsStep>(), advanceFunc, advanceFunc);

    // 3. Step: Store information about the spent output with each input of the transaction. Then store information about each output for future lookup.
    processQueue.addStep(std::make_unique<ConnectUTXOsStep>(utxoState), advanceFunc, advanceFunc);

    /* 4. Step: Parse the input script of each input based information about the associated output script.
     *    Then store information about each output address for future lookup. */
    processQueue.addStep(std::make_unique<GenerateScriptInputStep>(utxoAddressState), advanceFunc, advanceFunc);

    /* 5. Step: Attach a scriptNum to each script in the transaction. For address types which are
          deduplicated (Pubkey, ScriptHash, Multisig and their varients) use the previously allocated
          scriptNum if the address was seen before. Increment the scriptNum counter for newly seen addresses. */
    processQueue.addStep(std::make_unique<ProcessAddressesStep>(addressState), advanceFunc, advanceFunc);

    /* 6. Step: Record the scriptNum for each output for later reference. Assign each spent input with
     the scriptNum of the output its spending */
    processQueue.addStep(std::make_unique<RecordAddressesStep>(utxoScriptState), advanceFunc, advanceFunc);

    // 7. Step: Serialize transaction data, inputs, and outputs and write them to the txFile
    processQueue.addStep(std::make_unique<SerializeTransactionStep>(txFile, linkDataFile), advanceFunc, advanceFunc);

    // 8. Step: Save address data into files for the analysis library
    processQueue.addStep(std::make_unique<SerializeAddressesStep>(addressWriter), advanceFunc, serializeAddressAdvanceFunc);
    
    int64_t nextWaitCount = 0;
    
    std::vector<blocksci::RawBlock> blocksAdded;
    BlockFileReader<ParseTag> fileReader(config, blocks, currentTxNum);

    // Launch the importer in its own thread
    auto importer = std::async(std::launch::async, [&] {
        CompletionGuard guard(processQueue.importDone);
        auto loadFinishedTx = [&](RawTransaction *&tx) {
            return processQueue.finishedQueue.pop(tx);
        };

        
        // Function that adds transaction to the first queue of the processing pipeline
        auto outFunc = [&](RawTransaction *tx) {
            using namespace std::chrono_literals;
            // Add tx to the inputQueue of the first processing step, if it fails (queue is full), wait 5ms and try again
            while (!processQueue.inputQueue().push(tx)) {
                if (!processQueue.isRunning()) {
                    // Error: calculateHashesStep() finished before all items were queued
                    throw NextQueueFinishedEarlyException();
                }
                nextWaitCount++;
                std::this_thread::sleep_for(5ms);
            }
        };
        
        NewBlocksFiles files(config);

        // Call readNewBlock for every block, which triggers the processing pipeline for every transaction
        for (auto &block : blocks) {
            // Seek to the current block in the blkXXXXX.dat file and skip CblockHeader and transaction count integer
            fileReader.nextBlock(block, currentTxNum);

            auto newBlock = readNewBlock(currentTxNum, currentInputNum, currentOutputNum, block, fileReader, files, loadFinishedTx, outFunc, block.height >= config.dataConfig.chainConfig.segwitActivationHeight);
            blocksAdded.push_back(newBlock);
            currentTxNum += newBlock.txCount;
            currentInputNum += newBlock.inputCount;
            currentOutputNum += newBlock.outputCount;
        }
    });

    // Launch all processing steps as concurrent threads
    processQueue.run();

    // Wait for all processing step threads to complete
    importer.get();
    processQueue.waitForComplete();
    
//    std::cout << "generateTxesStep:" << nextWaitCount << "\n";
//    std::cout << "calculateHashesStep: " << calculateHashesStep.prevWaitCount << " " << calculateHashesStep.nextWaitCount << "\n";
//    std::cout << "generateScriptOutputsStep: " << generateScriptOutputsStep.prevWaitCount << " " << generateScriptOutputsStep.nextWaitCount << "\n";
//    std::cout << "connectUTXOsStep: " << connectUTXOsStep.prevWaitCount << " " << connectUTXOsStep.nextWaitCount << "\n";
//    std::cout << "generateScriptInputStep: " << generateScriptInputStep.prevWaitCount << " " << generateScriptInputStep.nextWaitCount << "\n";
//    std::cout << "processAddressStep: " << processAddressStep.prevWaitCount << " " << processAddressStep.nextWaitCount << "\n";
//    std::cout << "recordAddressesStep: " << recordAddressesStep.prevWaitCount << " " << recordAddressesStep.nextWaitCount << "\n";
//    std::cout << "serializeTransactionStep: " << serializeTransactionStep.prevWaitCount << " " << serializeTransactionStep.nextWaitCount << "\n";
//    std::cout << "serializeAddressStep: " << serializeAddressStep.prevWaitCount << " " << serializeAddressStep.nextWaitCount << "\n";
    
    return blocksAdded;
}

// addNewBlocksSingle has the same functionality as addNewBlocks except that it is single threaded instead of using multiple lock-free queues
//template <typename ParseTag>
//std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> blocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState) {
//
//    RawTransaction realTx;
//    auto loadFinishedTx = [&](RawTransaction *&tx) {
//        tx = &realTx;
//        return true;
//    };
//
//    FixedSizeFileWriter<blocksci::uint256> hashFile{blocksci::ChainAccess::txHashesFilePath(config.dataConfig.chainDirectory())};
//    AddressWriter addressWriter{config};
//
//    auto progressBar = blocksci::makeProgressBar(totalTxCount, [=](RawTransaction *tx) {
//        auto blockHeight = tx->blockHeight;
//        std::cout << ", Block " << blockHeight << "/" << maxBlockHeight;
//    });
//
//    FixedSizeFileWriter<OutputLinkData> linkDataFile(config.txUpdatesFilePath());
//    IndexedFileWriter<1> txFile(blocksci::ChainAccess::txFilePath(config.dataConfig.chainDirectory()));
//
//    auto outFunc = [&](RawTransaction *tx) {
//        calculateHash(*tx, hashFile);
//        connectUTXOs(*tx, utxoState);
//        generateScriptInput(*tx, utxoAddressState);
//        processAddresses(*tx, addressState);
//        recordAddresses(*tx, utxoScriptState);
//        serializeTransaction(*tx, txFile, linkDataFile);
//        serializeAddressess(*tx, addressWriter);
//        progressBar.update(tx->txNum - startingTxCount, tx);
//    };
//
//    BlockFileReader<ParseTag> fileReader(config, blocks, currentTxNum);
//    NewBlocksFiles files(config);
//
//    std::vector<blocksci::RawBlock> blocksAdded;
//    for (auto &block : blocks) {
//        fileReader.nextBlock(block, currentTxNum);
//        auto newBlock = readNewBlock(currentTxNum, currentInputNum, currentOutputNum, block, fileReader, files, loadFinishedTx, outFunc, block.height >= config.dataConfig.chainConfig.segwitActivationHeight);
//        blocksAdded.push_back(newBlock);
//        currentTxNum += newBlock.txCount;
//        currentInputNum += newBlock.inputCount;
//        currentOutputNum += newBlock.outputCount;
//    }
//
//    return blocksAdded;
//}

#ifdef BLOCKSCI_FILE_PARSER
template std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocks(const ParserConfiguration<FileTag> &config, std::vector<BlockInfo<FileTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
//template std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<FileTag> &config, std::vector<BlockInfo<FileTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
#endif
#ifdef BLOCKSCI_RPC_PARSER
template std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocks(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
//template std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocksSingle(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
#endif
