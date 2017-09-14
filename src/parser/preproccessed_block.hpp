//
//  preproccessed_block.hpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef preproccessed_block_hpp
#define preproccessed_block_hpp

#include "config.hpp"
#include "script_output.hpp"

#include <blocksci/bitcoin_uint256.hpp>

#include <boost/variant/variant.hpp>

#include <stdio.h>

struct InputInfo;
struct getrawtransaction_t;
struct vout_t;
struct vin_t;

namespace blocksci {
    struct RawTransaction;
    struct OutputPointer;
}



struct WitnessStackItem {
    uint32_t length;
    const char *itemBegin;
    
    #ifdef BLOCKSCI_FILE_PARSER
    WitnessStackItem(const char **buffer);
    #endif
};

struct RawInput {
    RawOutputPointer rawOutputPointer;
    uint32_t sequenceNum;
    const unsigned char *scriptBegin;
    uint32_t scriptLength;
    std::vector<WitnessStackItem> witnessStack;
    uint32_t linkedTxNum;
    blocksci::AddressType::Enum addressType;
    
    std::vector<unsigned char> scriptBytes;
    
    InputInfo getInfo(uint16_t i, bool isSegwit);
    
    RawInput(){}
    
    #ifdef BLOCKSCI_FILE_PARSER
    RawInput(const char **buffer);
    #endif
    
    #ifdef BLOCKSCI_RPC_PARSER
    RawInput(const vin_t &vin);
    #endif
};

struct RawOutput {
    uint64_t value;
    uint32_t scriptLength;
    const unsigned char *scriptBegin;
    ScriptOutputType scriptOutput;

    #ifdef BLOCKSCI_FILE_PARSER
    RawOutput(const char **buffer, bool witnessActivated);
    #endif
    
    #ifdef BLOCKSCI_RPC_PARSER
    RawOutput(const vout_t &vout, bool witnessActivated);
    #endif
    
    RawOutput(const std::vector<unsigned char> &scriptBytes, uint64_t value, bool witnessActivated);
    
    RawOutput(const ScriptOutputType &scriptOutput_, uint64_t value_, uint32_t scriptLength_) : value(value_), scriptLength(scriptLength_), scriptOutput(scriptOutput_) {}
    
    RawOutput(uint64_t value, uint32_t scriptLength, const char **buffer);
    
};

struct RawTransaction {
    uint32_t txNum;
    blocksci::uint256 hash;
    uint32_t sizeBytes;
    uint32_t locktime;
    int32_t version;
    uint32_t blockHeight;
    bool isSegwit;
    
    std::vector<RawInput> inputs;
    std::vector<RawOutput> outputs;
    
    
    RawTransaction() :
      txNum(0),
      hash(),
      sizeBytes(0),
      locktime(0),
      version(0),
      blockHeight(0) {}
    
    #ifdef BLOCKSCI_FILE_PARSER
    void load(const char **buffer, uint32_t blockHeight, bool witnessActivated);
    #endif
    
    #ifdef BLOCKSCI_RPC_PARSER
    void load(const getrawtransaction_t &txinfo, uint32_t blockHeight, bool witnessActivated);
    #endif
    
    blocksci::uint256 getHash(const InputInfo &info, int hashType) const;
    blocksci::RawTransaction getRawTransaction() const;
};


#endif /* preproccessed_block_hpp */
