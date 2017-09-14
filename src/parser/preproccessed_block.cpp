//
//  preproccessed_block.cpp
//  BlockParser2
//
//  Created by Harry Kalodner on 1/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "preproccessed_block.hpp"
#include "utilities.hpp"
#include "chain_index.hpp"
#include "script_input.hpp"
#include <blocksci/hash.hpp>
#include <blocksci/chain/transaction.hpp>

#ifdef BLOCKSCI_RPC_PARSER
#include <bitcoinapi/types.h>
#endif

#include <openssl/sha.h>

#include <iostream>

std::vector<unsigned char> hexStringToVec(const std::string scripthex) {
    std::vector<unsigned char> scriptBytes;
    for (unsigned int i = 0; i < scripthex.size(); i += 2) {
        std::string byteString = scripthex.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), NULL, 16);
        scriptBytes.push_back(byte);
    }
    return scriptBytes;
}

InputInfo RawInput::getInfo(uint16_t i, bool isSegwit) {
    if (scriptBytes.size() > 0) {
        return {i, scriptBytes.data(), static_cast<uint32_t>(scriptBytes.size()), witnessStack, isSegwit};
    } else {
        return {i, scriptBegin, scriptLength, witnessStack, isSegwit};
    }
}

ScriptOutputType getScriptOutput(const std::vector<unsigned char> &scriptBytes, bool witnessActivated) {
    return extractScriptData(scriptBytes.data(), scriptBytes.data() + scriptBytes.size(), witnessActivated);
}

RawOutput::RawOutput(const std::vector<unsigned char> &scriptBytes, uint64_t value, bool witnessActivated) : RawOutput(getScriptOutput(scriptBytes, witnessActivated), value, scriptBytes.size()) {}

#ifdef BLOCKSCI_FILE_PARSER
RawInput::RawInput(const char **buffer) {
    rawOutputPointer.hash = readNext<blocksci::uint256>(buffer);
    rawOutputPointer.outputNum = static_cast<uint16_t>(readNext<uint32_t>(buffer));
    scriptLength = readVariableLengthInteger(buffer);
    scriptBegin = reinterpret_cast<const unsigned char*>(*buffer);
    *buffer += scriptLength;
    sequenceNum = readNext<uint32_t>(buffer);
}

RawOutput::RawOutput(const char **buffer, bool witnessActivated) :
value(readNext<uint64_t>(buffer)),
scriptLength(readVariableLengthInteger(buffer)),
scriptBegin(reinterpret_cast<const unsigned char*>(*buffer)),
scriptOutput(extractScriptData(scriptBegin, scriptBegin + scriptLength, witnessActivated)) {
    *buffer += scriptLength;
}

WitnessStackItem::WitnessStackItem(const char **buffer) : length(readVariableLengthInteger(buffer)), itemBegin(*buffer)  {
    *buffer += length;
}

void RawTransaction::load(const char **buffer, uint32_t blockHeight_, bool witnessActivated) {
    isSegwit = witnessActivated;
    blockHeight = blockHeight_;
    SHA256_CTX sha256CTX;
    SHA256_Init(&sha256CTX);
    auto startPos = *buffer;
    SHA256_Update(&sha256CTX, *buffer, sizeof(int32_t));
    version = readNext<int32_t>(buffer);
    
    const char *inputStart = *buffer;
    auto inputCount = readVariableLengthInteger(buffer);
    
    bool isSegwit = false;
    if (inputCount == 0) {
        auto flag = readNext<uint8_t>(buffer);
        assert(flag == 1);
        isSegwit = true;
        inputStart = *buffer;
        inputCount = readVariableLengthInteger(buffer);
    }
    
    inputs.clear();
    inputs.reserve(inputCount);
    for (decltype(inputCount) i = 0; i < inputCount; i++) {
        inputs.emplace_back(buffer);
    }
    
    auto outputCount = readVariableLengthInteger(buffer);
    
    outputs.clear();
    outputs.reserve(outputCount);
    for (decltype(outputCount) i = 0; i < outputCount; i++) {
        outputs.emplace_back(buffer, witnessActivated);
    }
    
    SHA256_Update(&sha256CTX, inputStart, *buffer - inputStart);
    
    if (isSegwit) {
        for (decltype(inputCount) i = 0; i < inputCount; i++) {
            auto &input = inputs[i];
            uint32_t stackItemCount = readVariableLengthInteger(buffer);
            for (uint32_t j = 0; j < stackItemCount; j++) {
                input.witnessStack.emplace_back(buffer);
            }
        }
    }
    
    SHA256_Update(&sha256CTX, *buffer, sizeof(uint32_t));
    
    locktime = readNext<uint32_t>(buffer);
    sizeBytes = static_cast<uint32_t>(*buffer - startPos);
    
    SHA256_Final((unsigned char *)&hash, &sha256CTX);
    hash = sha256(reinterpret_cast<const uint8_t *>(&hash), sizeof(hash));
    // hash = doubleSha256(startPos, sizeBytes);
}

#endif

#ifdef BLOCKSCI_RPC_PARSER
RawInput::RawInput(const vin_t &vin) {
    rawOutputPointer = {blocksci::uint256S(vin.txid), static_cast<uint16_t>(vin.n)};
    sequenceNum = vin.sequence;
    scriptBytes = hexStringToVec(vin.scriptSig.hex);
}

RawOutput::RawOutput(const vout_t &vout, bool witnessActivated) : RawOutput(hexStringToVec(vout.scriptPubKey.hex), static_cast<uint64_t>(vout.value * 100000000), witnessActivated) {}

void RawTransaction::load(const getrawtransaction_t &txinfo, uint32_t blockHeight_, bool witnessActivated) {
    isSegwit = witnessActivated;
    blockHeight = blockHeight_;
    version = txinfo.version;
    locktime = txinfo.locktime;
    sizeBytes = txinfo.hex.size() / 2;
    unsigned int inputCount = txinfo.vin.size();
    inputs.clear();
    inputs.reserve(inputCount);
    for (unsigned int i = 0; i < inputCount; i++) {
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
        SHA256_Final((unsigned char *)&hash, &sha256);
        return hash;
    }
};

enum {
    SIGHASH_ALL = 1,
    SIGHASH_NONE = 2,
    SIGHASH_SINGLE = 3,
    SIGHASH_ANYONECANPAY = 0x80,
};

blocksci::uint256 RawTransaction::getHash(const InputInfo &info, int hashType) const {
    
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
            auto scriptCode = info.getScript();
            CScript::const_iterator it = scriptCode.begin();
            CScript::const_iterator itBegin = it;
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
                    s.serialize(&itBegin[0], it-itBegin-1);
                    itBegin = it;
                }
            }
            if (itBegin != scriptCode.end()) {
                s.serialize(&itBegin[0], it-itBegin);
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
        uint64_t value = outputs[nOutput].value;
        uint32_t scriptLength = outputs[nOutput].scriptLength;
        if (hashSingle && nOutput != info.inputNum) {
            value = -1;
            scriptLength = 0;
        }
        s.serialize(value);
        s.serializeCompact(scriptLength);
        if (scriptLength > 0) {
            s.serialize(outputs[nOutput].scriptBegin, scriptLength);
        }
    }
    s.serialize(locktime);
    return s.finalize();
}
