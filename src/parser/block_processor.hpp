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


#include <blocksci/util/file_mapper.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>

class BlockFileReaderBase {
public:
    virtual void nextTx(RawTransaction *tx, bool isSegwit) = 0;
    virtual void nextTxNoAdvance(RawTransaction *tx, bool isSegwit) = 0;
    virtual void receivedFinishedTx(RawTransaction *) = 0;
};

struct NewBlocksFiles {
    ArbitraryFileWriter blockCoinbaseFile;
    FixedSizeFileWriter<blocksci::RawBlock> blockFile;
    IndexedFileWriter<1> sequenceFile;
    
    NewBlocksFiles(const ParserConfigurationBase &config);
};

struct OutputLinkData {
    blocksci::OutputPointer pointer;
    uint32_t txNum;
};

std::vector<unsigned char> readNewBlock(uint32_t firstTxNum, const BlockInfoBase &block, BlockFileReaderBase &fileReader, NewBlocksFiles &files, const std::function<bool(RawTransaction *&tx)> &loadFunc, const std::function<void(RawTransaction *tx)> &outFunc);
void calculateHash(RawTransaction *tx, FixedSizeFileWriter<blocksci::uint256> &hashFile);
void generateScriptOutputs(RawTransaction *tx);
void connectUTXOs(RawTransaction *tx, UTXOState &utxoState);
void serializeTransaction(RawTransaction *tx, IndexedFileWriter<1> &txFile, FixedSizeFileWriter<OutputLinkData> &linkDataFile);
void generateScriptInput(RawTransaction *tx, UTXOAddressState &utxoAddressState);
void processAddresses(RawTransaction *tx, AddressState &addressState);
void recordAddresses(RawTransaction *tx, UTXOScriptState &state);
void serializeAddressess(RawTransaction *tx, AddressWriter &addressWriter);
void backUpdateTxes(const ParserConfigurationBase &config);


class BlockProcessor {
    
    uint32_t startingTxCount;
    uint32_t currentTxNum;
    uint32_t totalTxCount;
    uint32_t maxBlockHeight;

public:
    
    BlockProcessor(uint32_t startingTxCount, uint32_t totalTxCount, uint32_t maxBlockHeight);
    
    template <typename ParseTag>
    void addNewBlocks(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);

    template <typename ParseTag>
    void addNewBlocksSingle(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> nextBlocks, UTXOState &utxoState, UTXOAddressState &utxoAddressState, AddressState &addressState, UTXOScriptState &utxoScriptState);
};



#endif /* block_processor_hpp */
