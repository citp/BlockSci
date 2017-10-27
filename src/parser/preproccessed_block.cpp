//
//  preproccessed_block.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "preproccessed_block.hpp"
#include "chain_index.hpp"
#include "script_input.hpp"
#include "safe_mem_reader.hpp"

#include <blocksci/hash.hpp>
#include <blocksci/chain/transaction.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/types.h>
#endif

#include <openssl/sha.h>

#include <iostream>

using SequenceNum = uint32_t;
using Value = uint64_t;
using Locktime = uint32_t;

std::vector<unsigned char> hexStringToVec(const std::string scripthex);

std::vector<unsigned char> hexStringToVec(const std::string scripthex) {
    std::vector<unsigned char> scriptBytes;
    for (unsigned int i = 0; i < scripthex.size(); i += 2) {
        std::string byteString = scripthex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), NULL, 16));
        scriptBytes.push_back(byte);
    }
    return scriptBytes;
}

#ifdef BLOCKSCI_FILE_PARSER
RawInput::RawInput(SafeMemReader &reader) {
    rawOutputPointer.hash = reader.readNext<blocksci::uint256>();
    rawOutputPointer.outputNum = static_cast<uint16_t>(reader.readNext<uint32_t>());
    scriptLength = reader.readVariableLengthInteger();
    scriptBegin = reinterpret_cast<const unsigned char*>(reader.unsafePos());
    reader.advance(scriptLength);
    sequenceNum = reader.readNext<SequenceNum>();
}

RawOutput::RawOutput(SafeMemReader &reader, bool witnessActivated) {
    value = reader.readNext<Value>();
    scriptLength = reader.readVariableLengthInteger();
    scriptBegin = reinterpret_cast<const unsigned char*>(reader.unsafePos());
    scriptOutput = extractScriptData(scriptBegin, scriptBegin + scriptLength, witnessActivated);
    reader.advance(scriptLength);
}

WitnessStackItem::WitnessStackItem(SafeMemReader &reader) {
    length = reader.readVariableLengthInteger();
    itemBegin = reader.unsafePos();
    reader.advance(length);
}

void RawTransaction::load(SafeMemReader &reader, uint32_t txNum_, uint32_t blockHeight_, bool witnessActivated) {
    txNum = txNum_;
    isSegwit = witnessActivated;
    blockHeight = blockHeight_;
    auto startOffset = reader.offset();
    version = reader.readNext<uint32_t>();
    txHashStart = reader.unsafePos();
    
    auto inputCount = reader.readVariableLengthInteger();
    bool containsSegwit = false;
    if (inputCount == 0) {
        auto flag = reader.readNext<uint8_t>();
        assert(flag == 1);
        containsSegwit = true;
        txHashStart = reader.unsafePos();
        inputCount = reader.readVariableLengthInteger();
    }
    
    inputs.clear();
    inputs.reserve(inputCount);
    for (decltype(inputCount) i = 0; i < inputCount; i++) {
        inputs.emplace_back(reader);
    }
    
    auto outputCount = reader.readVariableLengthInteger();
    
    outputs.clear();
    outputs.reserve(outputCount);
    for (decltype(outputCount) i = 0; i < outputCount; i++) {
        outputs.emplace_back(reader, witnessActivated);
    }
    
    txHashLength = static_cast<uint32_t>(reader.unsafePos() - txHashStart);
    
    if (containsSegwit) {
        for (decltype(inputCount) i = 0; i < inputCount; i++) {
            auto &input = inputs[i];
            auto stackItemCount = reader.readVariableLengthInteger();
            for (uint32_t j = 0; j < stackItemCount; j++) {
                input.witnessStack.emplace_back(reader);
            }
        }
    }
    locktime = reader.readNext<Locktime>();
    sizeBytes = reader.offset() - startOffset;
    hash.SetNull();
}

TransactionHeader::TransactionHeader(SafeMemReader &reader) {
    auto startOffset = reader.offset();
    version = reader.readNext<uint32_t>();
    
    inputCount = reader.readVariableLengthInteger();
    bool containsSegwit = false;
    if (inputCount == 0) {
        auto flag = reader.readNext<uint8_t>();
        assert(flag == 1);
        containsSegwit = true;
        inputCount = reader.readVariableLengthInteger();
    }
    
    for (decltype(inputCount) i = 0; i < inputCount; i++) {
        reader.advance(sizeof(blocksci::uint256) + sizeof(uint32_t));
        auto scriptLength = reader.readVariableLengthInteger();
        reader.advance(scriptLength + sizeof(SequenceNum));
    }
    
 outputCount = reader.readVariableLengthInteger();
    for (decltype(outputCount) i = 0; i < outputCount; i++) {
        reader.advance(sizeof(Value));
        auto scriptLength = reader.readVariableLengthInteger();
        reader.advance(scriptLength);
    }
    
    if (containsSegwit) {
        for (decltype(inputCount) i = 0; i < inputCount; i++) {
            auto stackItemCount = reader.readVariableLengthInteger();
            for (uint32_t j = 0; j < stackItemCount; j++) {
                auto length = reader.readVariableLengthInteger();
                reader.advance(length);
            }
        }
    }
    locktime = reader.readNext<Locktime>();
    sizeBytes = reader.offset() - startOffset;
}

void RawTransaction::calculateHash() {
    if (hash.IsNull()) {
        SHA256_CTX sha256CTX;
        SHA256_Init(&sha256CTX);
        SHA256_Update(&sha256CTX, &version, sizeof(version));
        SHA256_Update(&sha256CTX, txHashStart, txHashLength);
        SHA256_Update(&sha256CTX, &locktime, sizeof(locktime));
        SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256CTX);
        hash = sha256(reinterpret_cast<const uint8_t *>(&hash), sizeof(hash));
    }
}

#endif

#ifdef BLOCKSCI_RPC_PARSER
RawInput::RawInput(const vin_t &vin) {
    rawOutputPointer = {blocksci::uint256S(vin.txid), static_cast<uint16_t>(vin.n)};
    sequenceNum = vin.sequence;
    scriptBytes = hexStringToVec(vin.scriptSig.hex);
    scriptLength = 0;
}

RawOutput::RawOutput(std::vector<unsigned char> scriptBytes_, uint64_t value_, bool witnessActivated) : scriptLength(0), scriptBytes(std::move(scriptBytes_)), value(value_)  {
    scriptOutput = extractScriptData(scriptBegin, scriptBegin + scriptLength, witnessActivated);
}

RawOutput::RawOutput(const vout_t &vout, bool witnessActivated) : RawOutput(hexStringToVec(vout.scriptPubKey.hex), static_cast<uint64_t>(vout.value * 100000000), witnessActivated) {}

void RawTransaction::load(const getrawtransaction_t &txinfo, uint32_t txNum_, uint32_t blockHeight_, bool witnessActivated) {
    txNum = txNum_;
    isSegwit = witnessActivated;
    blockHeight = blockHeight_;
    version = txinfo.version;
    locktime = static_cast<uint32_t>(txinfo.locktime);
    sizeBytes = static_cast<uint32_t>(txinfo.hex.size() / 2);
    auto inputCount = txinfo.vin.size();
    inputs.clear();
    inputs.reserve(inputCount);
    for (size_t i = 0; i < inputCount; i++) {
        inputs.emplace_back(txinfo.vin[i]);
    }
    auto outputCount = txinfo.vout.size();
    
    outputs.clear();
    outputs.reserve(outputCount);
    for (unsigned int i = 0; i < outputCount; i++) {
        outputs.emplace_back(txinfo.vout[i], witnessActivated);
    }
    hash = blocksci::uint256S(txinfo.txid);;
}

#endif

blocksci::RawTransaction RawTransaction::getRawTransaction() const {
    return {sizeBytes, locktime, static_cast<uint16_t>(inputs.size()), static_cast<uint16_t>(outputs.size())};
}

struct Serializer {
    SHA256_CTX sha256;
    
    Serializer() {
        SHA256_Init(&sha256);
    }
    
    template <typename T>
    void serialize(T t) {
        SHA256_Update(&sha256, reinterpret_cast<const char *>(&t), sizeof(t));
    }
    
    void serialize(const unsigned char *pos, size_t size) {
        SHA256_Update(&sha256, pos, size);
    }
    
    void serializeCompact(uint64_t t) {
        if (t < 253) {
            serialize(static_cast<uint8_t>(t));
        } else if (t <= std::numeric_limits<unsigned short>::max()) {
            serialize(uint8_t{253});
            serialize(static_cast<uint16_t>(t));
        } else if (t <= std::numeric_limits<unsigned int>::max()) {
            serialize(uint8_t{254});
            serialize(static_cast<uint32_t>(t));
        } else {
            serialize(uint8_t{255});
            serialize(static_cast<uint64_t>(t));
        }
        return;
    }
    
    blocksci::uint256 finalize() {
        blocksci::uint256 hash;
        SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
        return hash;
    }
};

enum {
    SIGHASH_ALL = 1,
    SIGHASH_NONE = 2,
    SIGHASH_SINGLE = 3,
    SIGHASH_ANYONECANPAY = 0x80,
};

blocksci::uint256 RawTransaction::getHash(const InputView &info, const InputScriptView &scriptView, int hashType) const {
    
    bool anyoneCanPay = !!(hashType & SIGHASH_ANYONECANPAY);
    bool hashSingle = (hashType & 0x1f) == SIGHASH_SINGLE;
    bool hashNone = (hashType & 0x1f) == SIGHASH_NONE;
    
    Serializer s;
    s.serialize(version);
    size_t nInputs = anyoneCanPay ? 1 :inputs.size();
    s.serializeCompact(nInputs);
    for (unsigned int nInput = 0; nInput < nInputs; nInput++) {
        if (anyoneCanPay) {
            nInput = info.inputNum;
        }
        // Serialize the prevout
        uint32_t outputNum = inputs[nInput].rawOutputPointer.outputNum;
        s.serialize(outputNum);
        // Serialize the script
        if (nInput != info.inputNum) {
            // Blank out other inputs' signatures
            s.serializeCompact(0);
        } else {
            auto scriptCode = scriptView.getScript();
            CScriptView::const_iterator it = scriptCode.begin();
            CScriptView::const_iterator itBegin = it;
            opcodetype opcode;
            unsigned int nCodeSeparators = 0;
            while (scriptCode.GetOp(it, opcode)) {
                if (opcode == OP_CODESEPARATOR)
                    nCodeSeparators++;
            }
            s.serializeCompact(scriptCode.size() - nCodeSeparators);
            it = itBegin;
            while (scriptCode.GetOp(it, opcode)) {
                if (opcode == OP_CODESEPARATOR) {
                    s.serialize(&itBegin[0], static_cast<size_t>(it-itBegin-1));
                    itBegin = it;
                }
            }
            if (itBegin != scriptCode.end()) {
                s.serialize(&itBegin[0], static_cast<size_t>(it-itBegin));
            }
        }
        // Serialize the nSequence
        if (nInput != info.inputNum && (hashSingle || hashNone)) {
            // let the others update at will
            s.serialize(uint32_t{0});
        } else {
            s.serialize(inputs[nInput].sequenceNum);
        }
    }
    size_t nOutputs = hashNone ? 0 : (hashSingle ? info.inputNum+1 : outputs.size());
    s.serializeCompact(nOutputs);
    for (unsigned int nOutput = 0; nOutput < nOutputs; nOutput++) {
        int64_t value = static_cast<int64_t>(outputs[nOutput].value);
        uint32_t scriptLength = outputs[nOutput].getScriptLength();
        if (hashSingle && nOutput != info.inputNum) {
            value = -1;
            scriptLength = 0;
        }
        s.serialize(value);
        s.serializeCompact(scriptLength);
        if (scriptLength > 0) {
            s.serialize(outputs[nOutput].getScriptBegin(), scriptLength);
        }
    }
    s.serialize(locktime);
    return s.finalize();
}
