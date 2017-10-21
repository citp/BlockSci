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

#include <blocksci/file_mapper.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/bitcoin_uint256.hpp>

struct RawTransaction;
class UTXOState;
class AddressState;
class AddressWriter;

bool checkSegwit(RawTransaction *tx);
void calculateHash(RawTransaction *tx, FixedSizeFileWriter<blocksci::uint256> &hashFile);
void connectUTXOs(RawTransaction *tx, UTXOState &utxoState);
std::vector<uint32_t> connectAddressess(RawTransaction *tx, AddressState &addressState, AddressWriter &addressWriter, blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, blocksci::RawTransaction> &txFile);

class BlockProcessor {
    
    uint32_t startingTxCount;
    uint32_t currentTxNum;
    uint32_t totalTxCount;
    uint32_t maxBlockHeight;

public:
    
    BlockProcessor(uint32_t startingTxCount, uint32_t totalTxCount, uint32_t maxBlockHeight);
    
    template <typename ParseTag>
    std::vector<uint32_t> addNewBlocks(const ParserConfiguration<ParseTag> &config, std::vector<BlockInfo<ParseTag>> nextBlocks, UTXOState &utxoState, AddressState &addressState);
};



#endif /* block_processor_hpp */
