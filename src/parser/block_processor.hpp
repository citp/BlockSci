//
//  block_processor.hpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/11/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef block_processor_hpp
#define block_processor_hpp

#include "config.hpp"
#include "parser_configuration.hpp"

#include <blocksci/chain/output_pointer.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/atomic.hpp>

#include <vector>
#include <stdio.h>

struct RawTransaction;
struct BlockInfo;
struct blockinfo_t;
class BitcoinAPI;

struct TxUpdate {
    blocksci::OutputPointer pointer;
    uint32_t linkedTxNum;
};

class BlockProcessor {
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> transaction_queue;
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> used_transaction_queue;
    
    boost::unordered_map<int, std::pair<boost::iostreams::mapped_file, uint32_t>> files;
    
    #ifdef BLOCKSCI_RPC_PARSER
    void loadTxRPC(RawTransaction *tx, const blockinfo_t &block, uint32_t txNum, BitcoinAPI & bapi);
    #endif
    
public:
    boost::atomic<bool> done;
    
    BlockProcessor();
    ~BlockProcessor();
    
    #ifdef BLOCKSCI_FILE_PARSER
    void readNewBlocks(FileParserConfiguration config, std::vector<BlockInfo> blocksToAdd, uint32_t startingTxCount);
    #endif
    #ifdef BLOCKSCI_RPC_PARSER
    void readNewBlocks(RPCParserConfiguration config, std::vector<blockinfo_t> blocksToAdd, uint32_t startingTxCount);
    #endif
    void processNewBlocks(ParserConfiguration config, uint32_t firstTxNum, uint32_t totalTxCount);
};



#endif /* block_processor_hpp */
