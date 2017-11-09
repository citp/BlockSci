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
#include "script_input.hpp"

#include <blocksci/bitcoin_uint256.hpp>

#include <boost/variant/variant.hpp>

struct getrawtransaction_t;
struct vout_t;
struct vin_t;
struct InputView;

namespace blocksci {
    struct RawTransaction;
    struct OutputPointer;
}

class SafeMemReader;

struct WitnessStackItem {
    uint32_t length;
    const char *itemBegin;
    
    #ifdef BLOCKSCI_FILE_PARSER
    WitnessStackItem(SafeMemReader &reader);
    #endif
};

struct RawInput {
private:
    const unsigned char *scriptBegin;
    uint32_t scriptLength;
    
    std::vector<unsigned char> scriptBytes;
    
public:
    
    RawOutputPointer rawOutputPointer;
    uint32_t sequenceNum;
    std::vector<WitnessStackItem> witnessStack;
    uint32_t linkedTxNum;
    blocksci::Address address;
    
    AnyScriptInput scriptInput;
    
    blocksci::OutputPointer getOutputPointer() const;
    
    CScriptView getScriptView() const {
        if (scriptLength == 0) {
            return CScriptView(scriptBytes.data(), scriptBytes.data() + scriptBytes.size());
        } else {
            return CScriptView(scriptBegin, scriptBegin + scriptLength);
        }
    }
    
    RawInput(){}
    
    #ifdef BLOCKSCI_FILE_PARSER
    RawInput(SafeMemReader &reader);
    #endif
    
    #ifdef BLOCKSCI_RPC_PARSER
    RawInput(const vin_t &vin);
    #endif
};

struct RawOutput {
private:
    const unsigned char *scriptBegin;
    uint32_t scriptLength;
    
    std::vector<unsigned char> scriptBytes;
public:
    uint64_t value;
    AnyScriptOutput scriptOutput;

    #ifdef BLOCKSCI_FILE_PARSER
    RawOutput(SafeMemReader &reader, bool witnessActivated);
    #endif
    
    #ifdef BLOCKSCI_RPC_PARSER
    RawOutput(const vout_t &vout, bool witnessActivated);
    RawOutput(std::vector<unsigned char> scriptBytes_, uint64_t value_, bool witnessActivated);
    #endif
    
    CScriptView getScriptView() const {
        if (scriptLength == 0) {
            return CScriptView(scriptBytes.data(), scriptBytes.data() + scriptBytes.size());
        } else {
            return CScriptView(scriptBegin, scriptBegin + scriptLength);
        }
    }
};

struct TransactionHeader {
    int32_t version;
    int inputCount;
    int outputCount;
    uint32_t sizeBytes;
    uint32_t locktime;
    TransactionHeader(SafeMemReader &reader);
};

struct RawTransaction {
    uint32_t txNum;
    blocksci::uint256 hash;
    uint32_t sizeBytes;
    uint32_t locktime;
    int32_t version;
    uint32_t blockHeight;
    bool isSegwit;
    const char *txHashStart;
    uint32_t txHashLength;
    
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
    void load(SafeMemReader &reader, uint32_t txNum, uint32_t blockHeight, bool witnessActivated);
    #endif
    
    #ifdef BLOCKSCI_RPC_PARSER
    void load(const getrawtransaction_t &txinfo, uint32_t txNum, uint32_t blockHeight, bool witnessActivated);
    #endif
    
    void calculateHash();
    
    blocksci::uint256 getHash(const InputView &info, const CScriptView &scriptView, int hashType) const;
    blocksci::RawTransaction getRawTransaction() const;
};


#endif /* preproccessed_block_hpp */
