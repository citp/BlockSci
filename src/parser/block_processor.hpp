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
#include "address_writer.hpp"

#include <blocksci/file_mapper.hpp>
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
class UTXOState;

struct RevealedScriptHash {
    uint32_t addressNum;
    uint32_t txNum;
    
    RevealedScriptHash(uint32_t addressNum_, uint32_t txNum_) : addressNum(addressNum_), txNum(txNum_) {}
};

class BlockProcessor {
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> hash_transaction_queue;
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> utxo_transaction_queue;
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> address_transaction_queue;
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> finished_transaction_queue;
    
    boost::unordered_map<int, std::pair<boost::iostreams::mapped_file, uint32_t>> files;
    
    void closeFinishedFiles(uint32_t txNum);
    #ifdef BLOCKSCI_RPC_PARSER
    void loadTxRPC(RawTransaction *tx, const blockinfo_t &block, uint32_t txNum, BitcoinAPI & bapi, bool witnessActivated);
    #endif
    
public:
    boost::atomic<bool> rawDone;
    boost::atomic<bool> hashDone;
    boost::atomic<bool> utxoDone;
    
    BlockProcessor();
    ~BlockProcessor();
    
    #ifdef BLOCKSCI_FILE_PARSER
    void readNewBlocks(FileParserConfiguration config, std::vector<BlockInfo> blocksToAdd, uint32_t startingTxCount);
    #endif
    #ifdef BLOCKSCI_RPC_PARSER
    void readNewBlocks(RPCParserConfiguration config, std::vector<blockinfo_t> blocksToAdd, uint32_t startingTxCount);
    #endif
    void calculateHashes(ParserConfiguration config);
    void processUTXOs(ParserConfiguration config, UTXOState &utxoState);
    std::vector<uint32_t> processAddresses(ParserConfiguration config, AddressState &addressState, uint32_t currentCount, uint32_t totalTxCount, uint32_t maxBlockHeight);
};



#endif /* block_processor_hpp */
