//
//  block_processor.hpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/11/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef block_processor_hpp
#define block_processor_hpp


#include "parser_fwd.hpp"
#include "parser_configuration.hpp"
#include "file_writer.hpp"

#include <blocksci/core/inout_pointer.hpp>
#include <blocksci/core/core_fwd.hpp>

class BlockFileReaderBase {
public:
    BlockFileReaderBase() = default;
    BlockFileReaderBase(const BlockFileReaderBase &) = default;
    virtual ~BlockFileReaderBase();

    virtual void nextTx(RawTransaction *tx, bool isSegwit) = 0;
    virtual void nextTxNoAdvance(RawTransaction *tx, bool isSegwit) = 0;
    virtual void receivedFinishedTx(RawTransaction *) = 0;
};

struct NewBlocksFiles {
    ArbitraryFileWriter blockCoinbaseFile;
    FixedSizeFileWriter<uint64_t> txFirstInput;
    FixedSizeFileWriter<uint64_t> txFirstOutput;
    FixedSizeFileWriter<int32_t> txVersionFile;
    FixedSizeFileWriter<uint16_t> inputSpentOutNumFile;
    FixedSizeFileWriter<uint32_t> inputSequenceFile;
    
    NewBlocksFiles(const ParserConfigurationBase &config);
};

struct OutputLinkData {
    blocksci::InoutPointer pointer;
    uint32_t txNum;
};

blocksci::RawBlock readNewBlock(uint32_t firstTxNum, uint64_t firstInputNum, uint64_t firstOutputNum, const BlockInfoBase &block, BlockFileReaderBase &fileReader, NewBlocksFiles &files, const std::function<bool(RawTransaction *&tx)> &loadFunc, const std::function<void(RawTransaction *tx)> &outFunc, bool isSegwit);

struct ProcessorStep {
    virtual std::function<void(RawTransaction &tx)> step() = 0;
    virtual ~ProcessorStep();
};

struct CalculateTxHashStep : public ProcessorStep {
    FixedSizeFileWriter<blocksci::uint256> &hashFile;
    
    CalculateTxHashStep(FixedSizeFileWriter<blocksci::uint256> &hashFile_) : hashFile(hashFile_) {}
    
    std::function<void(RawTransaction &tx)> step() override;
};

struct GenerateScriptOutputsStep : public ProcessorStep {
    std::function<void(RawTransaction &tx)> step() override;
};

struct StoreUTXOsStep : public ProcessorStep {
    UTXOState &utxoState;

    StoreUTXOsStep(UTXOState &utxoState_) : utxoState(utxoState_) {}

    std::function<void(RawTransaction &tx)> step() override;
};

struct ConnectUTXOsStep : public ProcessorStep {
    UTXOState &utxoState;
    
    ConnectUTXOsStep(UTXOState &utxoState_) : utxoState(utxoState_) {}
    
    std::function<void(RawTransaction &tx)> step() override;
};

struct StoreAddressDataStep : public ProcessorStep {
    UTXOAddressState &utxoAddressState;

    StoreAddressDataStep(UTXOAddressState &utxoAddressState_) : utxoAddressState(utxoAddressState_) {}

    std::function<void(RawTransaction &tx)> step() override;
};

struct GenerateScriptInputStep : public ProcessorStep {
    UTXOAddressState &utxoAddressState;
    
    GenerateScriptInputStep(UTXOAddressState &utxoAddressState_) : utxoAddressState(utxoAddressState_) {}
    
    std::function<void(RawTransaction &tx)> step() override;
};

struct ProcessAddressesStep : public ProcessorStep {
    AddressState &addressState;
    
    ProcessAddressesStep(AddressState &addressState_) : addressState(addressState_) {}
    
    std::function<void(RawTransaction &tx)> step() override;
};

struct RecordAddressesStep : public ProcessorStep {
    UTXOScriptState &state;
    
    RecordAddressesStep(UTXOScriptState &state_) : state(state_) {}
    
    std::function<void(RawTransaction &tx)> step() override;
};

struct LookupInputScriptNumStep : public ProcessorStep {
    UTXOScriptState &state;

    LookupInputScriptNumStep(UTXOScriptState &state_) : state(state_) {}

    std::function<void(RawTransaction &tx)> step() override;
};

struct SerializeTransactionStep : public ProcessorStep {
    IndexedFileWriter<1> &txFile;
    FixedSizeFileWriter<OutputLinkData> &linkDataFile;
    
    SerializeTransactionStep(IndexedFileWriter<1> &txFile_, FixedSizeFileWriter<OutputLinkData> &linkDataFile_) : txFile(txFile_), linkDataFile(linkDataFile_) {}
    
    std::function<void(RawTransaction &tx)> step() override;
};

struct SerializeNewScriptsStep : public ProcessorStep {
    AddressWriter &addressWriter;
    
    SerializeNewScriptsStep(AddressWriter &addressWriter_) : addressWriter(addressWriter_) {}
    
    std::function<void(RawTransaction &tx)> step() override;
};

struct SerializeExistingScriptsStep : public ProcessorStep {
    AddressWriter &addressWriter;

    SerializeExistingScriptsStep(AddressWriter &addressWriter_) : addressWriter(addressWriter_) {}

    std::function<void(RawTransaction &tx)> step() override;
};

void backUpdateTxes(const ParserConfigurationBase &config);


/** BlockProcessor handles parsing blocks and their transactions, inputs, outputs etc. using a processing pipeline */
class BlockProcessor {
    /** Blockchain-wide transaction numbers of the first and current transaction to process */
    uint32_t startingTxCount = 0;
    uint32_t currentTxNum = 0;

    /** Blockchain-wide current input and output numbers to process */
    uint64_t currentInputNum = 0;
    uint64_t currentOutputNum = 0;

    uint32_t totalTxCount = 0;
    blocksci::BlockHeight maxBlockHeight = 0;

public:
    
    BlockProcessor(uint32_t startingTxCount, uint64_t startingInputCount, uint64_t startingOutputCount, uint32_t totalTxCount, blocksci::BlockHeight maxBlockHeight);
    
    template <typename ParseTag>
    std::vector<blocksci::RawBlock> addNewBlocks(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);

    template <typename ParseTag>
    std::vector<blocksci::RawBlock> addNewBlocksSingle(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
};



#endif /* block_processor_hpp */
