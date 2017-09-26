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
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/atomic.hpp>

#include <unordered_map>
#include <vector>
#include <stdio.h>

struct RawTransaction;
struct BlockInfo;
struct blockinfo_t;
class BitcoinAPI;
class UTXOState;
class SafeMemReader;

class BlockProcessor {
    
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> hash_transaction_queue;
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> utxo_transaction_queue;
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<200000>> address_transaction_queue;
    boost::lockfree::spsc_queue<RawTransaction *, boost::lockfree::capacity<50000>> finished_transaction_queue;
    
    std::unordered_map<int, std::pair<SafeMemReader, uint32_t>> files;
    
    uint32_t startingTxCount;
    uint32_t currentTxNum;
    uint32_t totalTxCount;
    uint32_t maxBlockHeight;
    
    boost::atomic<bool> rawDone;
    boost::atomic<bool> hashDone;
    boost::atomic<bool> utxoDone;
    
    void closeFinishedFiles(uint32_t txNum);
    
    #ifdef BLOCKSCI_FILE_PARSER
    void readNewBlocks(FileParserConfiguration config, std::vector<BlockInfo> blocksToAdd);
    #endif
    #ifdef BLOCKSCI_RPC_PARSER
    void loadTxRPC(RawTransaction *tx, uint32_t txNum, const blockinfo_t &block, uint32_t txOffset, BitcoinAPI & bapi, bool witnessActivated);
    void readNewBlocks(RPCParserConfiguration config, std::vector<blockinfo_t> blocksToAdd);
    #endif
    void calculateHashes(ParserConfiguration config);
    void processUTXOs(ParserConfiguration config, UTXOState &utxoState);
    std::vector<uint32_t> processAddresses(ParserConfiguration config, AddressState &addressState);
    
    
public:
    
    BlockProcessor(uint32_t startingTxCount, uint32_t totalTxCount, uint32_t maxBlockHeight);
    ~BlockProcessor();
    
    template <typename ConfigType, typename BlockType>
    std::vector<uint32_t> addNewBlocks(const ConfigType &config, std::vector<BlockType> nextBlocks, UTXOState &utxoState, AddressState &addressState);
};



#endif /* block_processor_hpp */
