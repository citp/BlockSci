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

#include <blocksci/chain/inout_pointer.hpp>

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
    blocksci::ArbitraryFileWriter blockCoinbaseFile;
    blocksci::IndexedFileWriter<1> sequenceFile;
    
    NewBlocksFiles(const ParserConfigurationBase &config);
};

struct OutputLinkData {
    blocksci::OutputPointer pointer;
    uint32_t txNum;
};

blocksci::RawBlock readNewBlock(uint32_t firstTxNum, const BlockInfoBase &block, BlockFileReaderBase &fileReader, NewBlocksFiles &files, const std::function<bool(RawTransaction *&tx)> &loadFunc, const std::function<void(RawTransaction *tx)> &outFunc);
void calculateHash(RawTransaction &tx, blocksci::FixedSizeFileWriter<blocksci::uint256> &hashFile);
void generateScriptOutputs(RawTransaction &tx);
void connectUTXOs(RawTransaction &tx, UTXOState &utxoState);
void serializeTransaction(RawTransaction &tx, blocksci::IndexedFileWriter<1> &txFile, blocksci::FixedSizeFileWriter<OutputLinkData> &linkDataFile);
void generateScriptInput(RawTransaction &tx, UTXOAddressState &utxoAddressState);
void processAddresses(RawTransaction &tx, AddressState &addressState);
void recordAddresses(RawTransaction &tx, UTXOScriptState &state);
void serializeAddressess(RawTransaction &tx, AddressWriter &addressWriter);
void backUpdateTxes(const ParserConfigurationBase &config);


class BlockProcessor {
    uint32_t startingTxCount = 0;
    uint32_t currentTxNum = 0;
    uint32_t totalTxCount = 0;
    blocksci::BlockHeight maxBlockHeight = 0;

public:
    
    BlockProcessor(uint32_t startingTxCount, uint32_t totalTxCount, blocksci::BlockHeight maxBlockHeight);
    
    template <typename ParseTag>
    std::vector<blocksci::RawBlock> addNewBlocks(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);

    template <typename ParseTag>
    std::vector<blocksci::RawBlock> addNewBlocksSingle(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
};



#endif /* block_processor_hpp */
