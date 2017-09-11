//
//  chain_writer.hpp
//  BlockParser2
//
//  Created by Harry Kalodner on 12/29/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "chain_writer.hpp"
#include "parser_configuration.hpp"
#include "preproccessed_block.hpp"

#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>


#include <iostream>
#include <boost/filesystem.hpp>

ChainWriter::ChainWriter(const ParserConfiguration &config) : needsFlush(false), txFile(config.txFilePath()), txHashesFile(config.txHashesFilePath()), sequenceFile(config.sequenceFilePath()) {}

void ChainWriter::writeTransactionHeader(const blocksci::RawTransaction &tx) {
    txFile.writeIndexGroup();
    needsFlush |= txFile.write(tx);
    sequenceFile.writeIndexGroup();
}

void ChainWriter::writeTransactionHash(const blocksci::uint256 &hash) {
    txHashesFile.write(hash);
}

void ChainWriter::writeTransactionOutput(const blocksci::Inout &output) {
    needsFlush |= txFile.write(output);
}

void ChainWriter::writeTransactionInput(const blocksci::Inout &input, uint32_t sequenceNum) {
    needsFlush |= txFile.write(input);
    sequenceFile.write(sequenceNum);
}

void ChainWriter::finishTransaction() {
    if (needsFlush) {
        txFile.clearBuffer();
    }
}

void ChainWriter::truncate(uint32_t lastTxNum) {
    txFile.truncate(lastTxNum);
    txHashesFile.truncate(lastTxNum);
}

