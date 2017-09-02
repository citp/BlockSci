//
//  chain_writer.hpp
//  BlockParser2
//
//  Created by Harry Kalodner on 12/29/16.
//  Copyright © 2016 Harry Kalodner. All rights reserved.
//

#ifndef chain_writer_hpp
#define chain_writer_hpp

#include <blocksci/file_mapper.hpp>
#include <blocksci/uint256.hpp>

#include <stdio.h>

namespace blocksci {
    struct RawTransaction;
    struct Transaction;
    struct Inout;
    struct Input;
}

struct ParserConfiguration;
struct RawTransaction;

class ChainWriter {
public:
    blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::Transaction> txFile;
    blocksci::FixedSizeFileMapper<blocksci::uint256, boost::iostreams::mapped_file::readwrite> txHashesFile;
    blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, uint32_t> sequenceFile;
    
    ChainWriter(const ParserConfiguration &config);
    
    void writeTransactionHeader(const blocksci::RawTransaction &tx);
    void writeTransactionHash(const blocksci::uint256 &hash);
    void writeTransactionOutput(const blocksci::Inout &output);
    void writeTransactionInput(const blocksci::Inout &input, uint32_t sequenceNum);
    
    void truncate(uint32_t blockHeight);
};

#endif /* chain_writer_hpp */
