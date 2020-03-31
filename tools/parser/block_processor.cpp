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
    /** Map of (blkXXXXX.dat file number) -> pair(SafeMemReader for blkXXXXX.dat file, last tx number of this blkXXXXX.dat file) */
    std::unordered_map<int, std::pair<SafeMemReader, uint32_t>> files;

    /** Map of (blkXXXXX.dat file number) -> (last tx number of this blkXXXXX.dat file) */
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

/** Process single block and its transactions */
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

        // For every tx, write blockchain-wide number of the first tx input and output to file (FixedSizeFileMapper<uint64_t>)
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

/** 0. step of the processing pipeline
 * Calculate hash of transaction and write it to the hash file (chain/tx_hashes.dat) */
std::vector<std::function<void(RawTransaction &tx)>> CalculateTxHashStep::steps() {
    return {[&](RawTransaction &tx) {
        tx.calculateHash();
        hashFile.write(tx.hash);
    }};
}

/** 1. step of the processing pipeline
 * Parse the output scripts (into CScriptView) of the transaction in order to identify address types and extract relevant information. */
std::vector<std::function<void(RawTransaction &tx)>> GenerateScriptOutputsStep::steps() {
    return {[&](RawTransaction &tx) {
        tx.scriptOutputs.clear();
        tx.scriptOutputs.reserve(tx.outputs.size());
        for (auto &output : tx.outputs) {
            // 2nd param assumes p2sh is always active
            // TODO: Add flag to disable p2sh
            tx.scriptOutputs.emplace_back(output.getScriptView(), true, tx.isSegwit);
        }
    }};
}

/** 2. step of the processing pipeline
 * Store information about each output for future lookup. */
std::vector<std::function<void(RawTransaction &tx)>> StoreUTXOsStep::steps() {
    return {[&](RawTransaction &tx) {
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
    }};
}

/** 3. step of the processing pipeline
 * Connect each input with the UTXO it is spending. */
std::vector<std::function<void(RawTransaction &tx)>> ConnectUTXOsStep::steps() {
    return {[&](RawTransaction &tx) {
        for (auto &input : tx.inputs) {
            // remove the output that this input spends from the UTXOState and assign it to the input
            input.utxo = utxoState.erase(input.rawOutputPointer);
        }
    }};
}



/** 4. step of the processing pipeline
 * Parse the input script of each input based information about the associated output script.
 * Then store information about each output address for future lookup. */
std::vector<std::function<void(RawTransaction &tx)>> StoreAddressDataStep::steps() {
    return {[&](RawTransaction &tx) {
        uint16_t i = 0;
        for (auto &scriptOutput : tx.scriptOutputs) {
            utxoAddressState.addOutput(AnySpendData{scriptOutput}, {tx.txNum, i});
            i++;
        }
    }};
}

/** 5. step of the processing pipeline
 * Parse the input script of each input based information about the associated output script. */
std::vector<std::function<void(RawTransaction &tx)>> GenerateScriptInputStep::steps() {
    return {[&](RawTransaction &tx) {
        tx.scriptInputs.clear();
        tx.scriptInputs.reserve(tx.inputs.size());
        uint16_t i = 0;
        for (auto &input : tx.inputs) {
            InputView inputView(i, tx.txNum, input.getWitnessStack(), tx.isSegwit);
            auto spendData = utxoAddressState.spendOutput(input.getOutputPointer(), input.utxo.type);
            tx.scriptInputs.emplace_back(inputView, input.getScriptView(), tx, spendData);
            i++;
        }
    }};
}

/** 6. step of the processing pipeline
 * Attach a scriptNum to each script in the transaction. For address types which are
 * deduplicated (Pubkey, ScriptHash, Multisig and their varients) use the previously allocated
 * scriptNum if the address was seen before. Increment the scriptNum counter for newly seen addresses. */
std::vector<std::function<void(RawTransaction &tx)>> ProcessAddressesStep::steps() {
    return {[&](RawTransaction &tx) {
        for (auto &scriptOutput : tx.scriptOutputs) {
            scriptOutput.resolve(addressState);
        }
        for (auto &scriptInput : tx.scriptInputs) {
            scriptInput.process(addressState);
        }
    }};
}

/** 7. step of the processing pipeline
 * Record the scriptNum for each output. */
std::vector<std::function<void(RawTransaction &tx)>> RecordAddressesStep::steps() {
    return {[&](RawTransaction &tx) {
        uint16_t i = 0;
        for (auto &scriptOutput : tx.scriptOutputs) {
            auto scriptNum = scriptOutput.address().scriptNum;
            assert(scriptNum > 0);
            state.add({tx.txNum, i}, scriptNum);
            i++;
        }
    }};
}

/** 8. step of the processing pipeline
 * Assign each spent input with the scriptNum of the output its spending. */
std::vector<std::function<void(RawTransaction &tx)>> LookupInputScriptNumStep::steps() {
    return {[&](RawTransaction &tx) {
        for (size_t i = 0; i < tx.inputs.size(); i++) {
            auto &input = tx.inputs[i];
            auto &scriptInput = tx.scriptInputs[i];
            auto scriptNum = state.erase(input.getOutputPointer());
            assert(scriptNum > 0);
            scriptInput.setScriptNum(scriptNum);
        }
    }};
}

/** 9. step of the processing pipeline
 * Serialize transaction data, inputs, and outputs and write them to the txFile */
std::vector<std::function<void(RawTransaction &tx)>> SerializeTransactionStep::steps() {
    return {[&](RawTransaction &tx) {
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
    }};
}

/** 10. step of the processing pipeline
 * Save address data into files for the analysis library */
std::vector<std::function<void(RawTransaction &tx)>> SerializeAddressesStep::steps() {
    return {[&](RawTransaction &tx) {        
        for (auto &scriptOutput : tx.scriptOutputs) {
            if (scriptOutput.isNew()) {
                // serialize new script to file
                addressWriter.serializeNewOutput(scriptOutput, tx.txNum, true);
            }
        }
        
        for (size_t i = 0; i < tx.inputs.size(); i++) {
            auto &input = tx.inputs[i];
            auto &scriptInput = tx.scriptInputs[i];
            addressWriter.serializeWrappedScript(scriptInput, tx.txNum, input.utxo.txNum);
        }
    }, [&](RawTransaction &tx) {
        // updates the seenTopLevel flag for outputs that have only been seen wrapped in inputs so far
        for (auto &scriptOutput : tx.scriptOutputs) {
            if (!scriptOutput.isNew()) {
                addressWriter.serializeExistingOutput(scriptOutput, true);
            }
        }
        
        for (size_t i = 0; i < tx.inputs.size(); i++) {
            auto &input = tx.inputs[i];
            auto &scriptInput = tx.scriptInputs[i];
            addressWriter.serializeInput(scriptInput, tx.txNum, input.utxo.txNum);
        }
    }};
}

/**
 Updated outputs that have been spent.
 */
void backUpdateTxes(const ParserConfigurationBase &config) {
    std::vector<OutputLinkData> updates;
    
    std::cout << "Updating spent outputs" << std::endl;
    
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

/* Single-producer/single-consumer fifo queues, pushing and popping is wait-free
 * size of the ringbuffer is specified by boost::lockfree::capacity<>
 */
using TxQueue = boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<10000>>;

using DiscardCheckFunc = std::function<bool(RawTransaction &)>;

struct StepNum {
    size_t threadNum;
    size_t subStepNum;
    
    bool operator==(const StepNum &o) const {
        return threadNum == o.threadNum && subStepNum == o.subStepNum;
    }
};

namespace std {
    template <>
    struct hash<StepNum> {
        size_t operator()(const StepNum &s) const {
            return s.threadNum + (s.subStepNum << 32);
        }
    };
}

std::ostream& operator<<(std::ostream& ofs, const StepNum& p){
    return ofs << "(" << p.threadNum << ", " << p.subStepNum << ")";
}


struct QueueStage {
    virtual bool processNext() = 0;
    virtual void complete() = 0;
    virtual ~QueueStage() = default;
    
    std::atomic<bool> *prevDone = nullptr;
    
    std::atomic<bool> isDone{false};
    TxQueue inputQueue;
    
    TxQueue *nextQueue;
    std::atomic<bool> *nextDone = nullptr;
    
    int64_t nextWaitCount;
    StepNum stepNum;
    
    void push(RawTransaction *tx) {
        using namespace std::chrono_literals;
        while (!nextQueue->push(tx)) {
            if (nextDone && *nextDone) {
                // Error: next ProcessStep finished before all items were queued
                throw NextQueueFinishedEarlyException();
            }
            nextWaitCount++;
            std::this_thread::sleep_for(5ms);
        }
    }
    
    void linkBack(QueueStage &prevStage) {
        // Link the queue- and done-pointers for the previous queue to this object's variables
        prevStage.nextQueue = &inputQueue;
        prevStage.nextDone = &isDone;
        prevDone = &prevStage.isDone;
    }
    
    bool prevFinished() {
        assert(prevDone != nullptr);
        return *prevDone;
    }
};

class ProcessSubStep : public QueueStage {
public:
    std::function<void(RawTransaction &)> func;
    DiscardCheckFunc shouldDiscard;
    bool discardIfFull;
    
    ProcessSubStep(std::function<void(RawTransaction &)> func_, const DiscardCheckFunc &shouldDiscard_, bool discardIfFull_) : func(std::move(func_)), shouldDiscard(shouldDiscard_), discardIfFull(discardIfFull_) {}
    
    // inputProcessingDone
    bool processNext() override {
        if (inputQueue.read_available() && (discardIfFull || nextQueue->write_available() > 0)) {
            RawTransaction *rawTx = nullptr;
            inputQueue.pop(rawTx);
//            {
//                static std::mutex m;
//                std::lock_guard<std::mutex> lock(m);
//                std::cout << "Step: " << stepNum << " processing tx " << rawTx->txNum << "\n";
//            }
            assert(rawTx != nullptr);
            // Execute processing step on rawTx, eg. calculateHashesFunc or connectUTXOsFunc
            func(*rawTx);
            // Check if advanceFunc is successful before further processing the pipeline
            if (nextQueue->write_available() == 0 || shouldDiscard(*rawTx)) {
                delete rawTx;
            } else {
                push(rawTx);
            }
            return true;
        } else {
            return false;
        }
    }
    
    void complete() override {
        
    }
};

ProcessorStep::~ProcessorStep() = default;

struct TxHoldSubStep : public QueueStage {
    std::vector<RawTransaction *> heldTransactions;
    
    int64_t prevWaitCount = 0;
    
    TxHoldSubStep() {}
    
    ~TxHoldSubStep() override {
        assert(heldTransactions.empty());
    }
    
    void emptyQueue() {
        using namespace std::chrono_literals;
        for (auto tx : heldTransactions) {
            push(tx);
        }
        heldTransactions.clear();
    }
    
    bool processNext() override {
        RawTransaction *rawTx = nullptr;
        inputQueue.pop(rawTx);
        if (rawTx) {
            if (heldTransactions.size() == 0 || heldTransactions.back()->blockHeight == rawTx->blockHeight) {
                heldTransactions.push_back(rawTx);
            } else {
                emptyQueue();
                heldTransactions.push_back(rawTx);
            }
            return true;
        } else {
            return false;
        }
    }
    
    void complete() override {
        emptyQueue();
    }
};

class ProcessStep {
public:
    std::unique_ptr<ProcessorStep> func;
    std::vector<std::unique_ptr<QueueStage>> stages;
    
    int64_t prevWaitCount = 0;
    
    // AdvanceFunc
    ProcessStep(std::unique_ptr<ProcessorStep> func_, std::vector<std::unique_ptr<QueueStage>> stages_) : func(std::move(func_)), stages(std::move(stages_)) {
    }

    bool anyNotDone() {
        for (auto &stage : stages) {
            if (!stage->prevFinished() || !stage->inputQueue.empty()) {
                return true;
            }
        }
        return false;
    }
    
    void doAll() {
        bool success = true;
        while (success) {
            success = false;
            for (auto &stage : stages) {
                success |= stage->processNext();
            }
        }
        
        for (auto &stage : stages) {
            if (stage->prevFinished() && stage->inputQueue.empty()) {
                stage->complete();
                stage->isDone = true;
            }
        }
    }
    // inputProcessingDone
    void run() {
        using namespace std::chrono_literals;
        // CompletionGuard sets isDone to true in its destructor that is called at the end of this operator() method
        std::list<CompletionGuard> guards;
        for (auto &stage : stages) {
            guards.emplace_back(stage->isDone);
        }
        
        // Consume queued items as long as the previous processing step has not finished
        while (anyNotDone()) {
            doAll();
            prevWaitCount++;
            std::this_thread::sleep_for(5ms);
        }

        // Last call to consume queued items to catch last items
        doAll();
        for (auto &stage : stages) {
            stage->complete();
        }
    }
};

ProcessStep makeStandardProcessStep(std::unique_ptr<ProcessorStep> && func, const DiscardCheckFunc &advanceFuncFirst, const DiscardCheckFunc &advanceFuncSecond, bool discardIfFullFirst = false, bool discardIfFullSecond = false) {
    std::vector<std::unique_ptr<QueueStage>> subSteps;
    auto steps = func->steps();
    for (size_t i = 0; i < steps.size(); i++) {
        if (i == steps.size() - 1) {
            subSteps.push_back(std::make_unique<ProcessSubStep>(steps[i], advanceFuncSecond, discardIfFullSecond));
        } else {
            subSteps.push_back(std::make_unique<ProcessSubStep>(steps[i], advanceFuncFirst, discardIfFullFirst));
        }
    }
    return {std::move(func), std::move(subSteps)};
}

ProcessStep makeHoldTxStep() {
    std::vector<std::unique_ptr<QueueStage>> subSteps;
    subSteps.push_back(std::make_unique<TxHoldSubStep>());
    
    std::unique_ptr<ProcessorStep> emptyStep;
    return {std::move(emptyStep), std::move(subSteps)};
}

struct ProcessStepQueue {
    
    std::atomic<bool> importDone{false};
    std::atomic<bool> processingDone{false};
    
    // Queue for RawTransaction objects that have gone through the entire processing pipeline
    TxQueue finishedQueue;
    
    QueueStage *firstStage;
    
    std::vector<ProcessStep> steps;
    std::vector<std::future<void>> futures;
    
    ProcessStepQueue() {}
    
    void addStep(ProcessStep && step) {
        steps.emplace_back(std::move(step));
    }
    
    void setStepOrder(const std::vector<StepNum> &subStepList) {
        std::unordered_set<StepNum> allSubSteps;
        size_t j = 0;
        for (auto &step : steps) {
            for (size_t i = 0; i < step.stages.size(); i++) {
                allSubSteps.insert({j, i});
            }
            j++;
        }
        
        // Make sure that subStepList includes every step once
        for (const auto &stepNum : subStepList) {
            auto it = allSubSteps.find(stepNum);
            if (it != allSubSteps.end()) {
                allSubSteps.erase(it);
            } else {
                throw std::runtime_error("Queue step order cannot include substep multiple times");
            }
        }
        
        if (!allSubSteps.empty()) {
            throw std::runtime_error("Queue step order list must include all defined steps");
        }
        
        QueueStage *prevStage = nullptr;
        for (const auto &stepNum : subStepList) {
            auto &stage = steps[stepNum.threadNum].stages[stepNum.subStepNum];
            stage->stepNum = stepNum;
            if (prevStage != nullptr) {
                stage->linkBack(*prevStage);
            } else {
                stage->prevDone = &importDone;
            }
            prevStage = stage.get();
        }
        
        auto lastStepNum = subStepList.back();
        auto &lastStage = steps[lastStepNum.threadNum].stages[lastStepNum.subStepNum];
        
        // Add finishedQueue as the last queue after the last actual processing step
        lastStage->nextQueue = &finishedQueue;
        lastStage->nextDone = &processingDone;
        
        auto firstStepNum = subStepList.front();
        firstStage = steps[firstStepNum.threadNum].stages[firstStepNum.subStepNum].get();
    }
    
    TxQueue &inputQueue() {
        assert(firstStage != nullptr);
        return firstStage->inputQueue;
    }
    
    bool isRunning() {
        assert(firstStage != nullptr);
        return !firstStage->isDone;
    }
    
    void run() {
        for (auto &step : steps) {
            futures.push_back(std::async(std::launch::async, [&]() {
                step.run();
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

    auto discardFunc = [](RawTransaction &) { return false; };
    
    auto progressBar = blocksci::makeProgressBar(totalTxCount, [=](RawTransaction &tx) {
        std::cout << ", Block " << tx.blockHeight << "/" << maxBlockHeight;
    });
    
    /* Advance function of the last step
     * Optimization: Only push tx to finished_transaction_queue if it meets certain conditions, otherwise de-allocate it */
    
    auto serializeAddressDiscardFunc = [&](RawTransaction &tx) {
        progressBar.update(tx.txNum - startingTxCount, tx);
        return tx.realSize >= 800;
    };
    
    // Definition of all ProcessStep objects for the processing pipeline
    ProcessStepQueue processQueue;
    
    // 0. Step: Calculate hash of transaction and write it to the hash file (chain/tx_hashes.dat)
    processQueue.addStep(makeStandardProcessStep(std::make_unique<CalculateTxHashStep>(txHashFile), discardFunc, discardFunc));

    // 1. Step: Parse the output scripts (into CScriptView) of the transaction in order to identify address types and extract relevant information.
    processQueue.addStep(makeStandardProcessStep(std::make_unique<GenerateScriptOutputsStep>(), discardFunc, discardFunc));

    // 2. Step: Store information about each output for future lookup.
    processQueue.addStep(makeStandardProcessStep(std::make_unique<StoreUTXOsStep>(utxoState), discardFunc, discardFunc));

    // 3. Step: Connect each input with the UTXO it spends.
    processQueue.addStep(makeStandardProcessStep(std::make_unique<ConnectUTXOsStep>(utxoState), discardFunc, discardFunc));

    // 4. Step: Store information about each output address for future lookup.
    processQueue.addStep(makeStandardProcessStep(std::make_unique<StoreAddressDataStep>(utxoAddressState), discardFunc, discardFunc));

    // 5. Step: Parse the input script of each input based information about the associated output script.
    processQueue.addStep(makeStandardProcessStep(std::make_unique<GenerateScriptInputStep>(utxoAddressState), discardFunc, discardFunc));

    /* 6. Step: Attach a scriptNum to each script in the transaction. For address types which are
          deduplicated (Pubkey, ScriptHash, Multisig and their varients) use the previously allocated
          scriptNum if the address was seen before. Increment the scriptNum counter for newly seen addresses. */
    processQueue.addStep(makeStandardProcessStep(std::make_unique<ProcessAddressesStep>(addressState), discardFunc, discardFunc));

    // 7. Step: Record the scriptNum for each output for later reference.
    processQueue.addStep(makeStandardProcessStep(std::make_unique<RecordAddressesStep>(utxoScriptState), discardFunc, discardFunc));

    // 8. Step: Assign each spent input with the scriptNum of the output its spending
    processQueue.addStep(makeStandardProcessStep(std::make_unique<LookupInputScriptNumStep>(utxoScriptState), discardFunc, discardFunc));

    // 9. Step: Serialize transaction data, inputs, and outputs and write them to the txFile
    processQueue.addStep(makeStandardProcessStep(std::make_unique<SerializeTransactionStep>(txFile, linkDataFile), discardFunc, discardFunc));

    // 10. Step: Save address data into files for the analysis library
    processQueue.addStep(makeStandardProcessStep(std::make_unique<SerializeAddressesStep>(addressWriter), discardFunc, serializeAddressDiscardFunc, false, true));
    
    // Two hold stages for ATOR
    processQueue.addStep(makeHoldTxStep()); // 11
    processQueue.addStep(makeHoldTxStep()); // 12
    
    processQueue.setStepOrder({
        {0, 0}, // calculate tx hash
        {1, 0}, // parse outputs into CScriptView
        {2, 0}, // store UTXOs
        {4, 0}, // store scripts
        {11, 0}, // ---
        {3, 0}, // connect inputs to outputs
        {5, 0}, // parse input scripts using output data
        {6, 0}, // attach scriptNum to outputs and inputs
        {7, 0}, // store scriptNum of each output for lookup
        {10, 0}, // serialize new scripts in outputs and wrapped inputs
        {12, 0}, // ---
        {8, 0}, // look up scriptNum of each input from output
        {9, 0}, // serialize transaction data
        {10, 1}  // update scripts
    });
    
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

    return blocksAdded;
}


#ifdef BLOCKSCI_FILE_PARSER
template std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocks(const ParserConfiguration<FileTag> &config, std::vector<BlockInfo<FileTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
#endif
#ifdef BLOCKSCI_RPC_PARSER
template std::vector<blocksci::RawBlock> BlockProcessor::addNewBlocks(const ParserConfiguration<RPCTag> &config, std::vector<BlockInfo<RPCTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
#endif
