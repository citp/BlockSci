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
#include "blockchain_state.hpp"

#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>


#include <iostream>
#include <boost/filesystem.hpp>

ChainWriter::ChainWriter(const ParserConfiguration &config) : txFile(config.txFilePath()), txHashesFile(config.txHashesFilePath()), sequenceFile(config.sequenceFilePath()), txVpubFile(config.txVpubFilePath()) {}

void ChainWriter::writeTransactionHeader(const blocksci::RawTransaction &tx) {
    txFile.writeIndexGroup();
    txFile.write(tx);
    sequenceFile.writeIndexGroup();
}

void ChainWriter::writeTransactionHash(const blocksci::uint256 &hash) {
    txHashesFile.write(hash);
}

void ChainWriter::writeTransactionUintIndex() {
    txVpubFile.writeIndexGroup();
}

void ChainWriter::writeTransactionUint(uint64_t vpub) {
    txFile.write(vpub);
}

void ChainWriter::writeTransactionOutput(const blocksci::Inout &output) {
    txFile.write(output);
}

void ChainWriter::writeTransactionInput(const blocksci::Inout &input, uint32_t sequenceNum) {
    txFile.write(input);
    sequenceFile.write(sequenceNum);
}

void ChainWriter::truncate(uint32_t lastTxNum) {
    txFile.truncate(lastTxNum);
    txHashesFile.truncate(lastTxNum);
}

