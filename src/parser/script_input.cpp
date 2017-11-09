//
//  script_input.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_input.hpp"
#include "address_writer.hpp"
#include "preproccessed_block.hpp"
#include "output_spend_data.hpp"
#include <blocksci/scripts/bitcoin_pubkey.hpp>

#include <secp256k1.h>

#include <iostream>

struct ScriptInputGenerator : public boost::static_visitor<ScriptInputType> {
    const InputView &inputView;
    const CScriptView &scriptView;
    const RawTransaction &tx;
    ScriptInputGenerator(const InputView &inputView_, const CScriptView &scriptView_, const RawTransaction &tx_) : inputView(inputView_), scriptView(scriptView_), tx(tx_) {}
    
    template <blocksci::AddressType::Enum type>
    ScriptInputType operator()(const SpendData<type> &spendData) const {
        return ScriptInput<type>(inputView, scriptView, tx, spendData);
    }
};

struct ScriptInputProcessor : public boost::static_visitor<void> {
    AddressState &state;
    ScriptInputProcessor(AddressState &state_) : state(state_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(ScriptInput<type> &input) const {
        input.process(state);
    }
};

struct ScriptInputSerializer : public boost::static_visitor<ProcessedInput> {
    uint32_t scriptNum;
    AddressWriter &addressWriter;
    ScriptInputSerializer(uint32_t scriptNum_, AddressWriter &addressWriter_) : scriptNum(scriptNum_), addressWriter(addressWriter_) {}
    template <blocksci::AddressType::Enum type>
    ProcessedInput operator()(ScriptInput<type> &input) const {
        return input.serialize(scriptNum, addressWriter);
    }
};

struct ScriptInputChecker : public boost::static_visitor<void> {
    const AddressState &state;
    ScriptInputChecker(const AddressState &state_) : state(state_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(ScriptInput<type> &input) const {
        input.check(state);
    }
};

AnyScriptInput::AnyScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AnySpendData &spendData) : wrapped(boost::apply_visitor(ScriptInputGenerator{inputView, scriptView, tx}, spendData.wrapped)) {}

void AnyScriptInput::process(AddressState &state) {
    ScriptInputProcessor visitor{state};
    boost::apply_visitor(visitor, wrapped);
}

ProcessedInput AnyScriptInput::serialize(uint32_t scriptNum, AddressWriter &addressWriter) {
    ScriptInputSerializer visitor{scriptNum, addressWriter};
    return boost::apply_visitor(visitor, wrapped);
}

void AnyScriptInput::check(const AddressState &state) {
    ScriptInputChecker visitor{state};
    boost::apply_visitor(visitor, wrapped);
}


std::pair<AnyScriptOutput, AnyScriptInput> p2shGenerate(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::SCRIPTHASH> &) {
    CScriptView::const_iterator pc1 = scriptView.begin();
    CScriptView::const_iterator prevpc = scriptView.begin();
    CScriptView::const_iterator prevprevpc = scriptView.begin();
    opcodetype opcode1;
    boost::iterator_range<const unsigned char *> vch1;
    
    boost::iterator_range<const unsigned char *> lastScript;
    while(true) {
        prevprevpc = prevpc;
        prevpc = pc1;
        lastScript = vch1;
        if(!scriptView.GetOp(pc1, opcode1, vch1)) {
            break;
        }
    }
    
    CScriptView::const_iterator begin = scriptView.begin();
    auto wrappedInputBegin = &*begin;
    auto wrappedInputLength = static_cast<uint32_t>(std::distance(begin, prevprevpc));
    
    CScriptView wrappedOutputScript(lastScript.begin(), lastScript.end());
    auto wrappedScriptOutput = AnyScriptOutput(wrappedOutputScript, inputView.witnessActivated);
    
    CScriptView p2shScriptView{wrappedInputBegin, wrappedInputBegin + wrappedInputLength};
    auto spendData = AnySpendData(wrappedScriptOutput);
    auto wrappedScriptInput = AnyScriptInput(inputView, p2shScriptView, tx, spendData);
    return std::make_pair(wrappedScriptOutput, wrappedScriptInput);
}

ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::ScriptInput(const InputView &inputView, const CScriptView &scriptView,const std::pair<AnyScriptOutput, AnyScriptInput> &data) : ScriptInputBase(inputView, scriptView), wrappedScriptOutput(data.first), wrappedScriptInput(data.second) {}

ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::SCRIPTHASH> &spendData) : ScriptInput(inputView, scriptView, p2shGenerate(inputView, scriptView, tx, spendData)) {}

void ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::process(AddressState &state) {
    wrappedScriptOutput.resolve(state);
    wrappedScriptInput.process(state);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    bool firstSpend = writer.serialize(*this, scriptNum);
    ProcessedInput processedInput = wrappedScriptInput.serialize(wrappedScriptOutput.address().scriptNum, writer);
    if (firstSpend) {
        processedInput.push_back(scriptNum);
    }
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::check(const AddressState &state) {
    wrappedScriptOutput.check(state);
    wrappedScriptInput.check(state);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::PUBKEY>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    writer.serialize(*this, scriptNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::PUBKEYHASH> &) : ScriptInputBase(inputView, scriptView) {
    if (scriptView.size() > 0) {
        auto pc = scriptView.begin();
        opcodetype opcode;
        boost::iterator_range<const unsigned char *> vchSig;
        scriptView.GetOp(pc, opcode, vchSig);
        scriptView.GetOp(pc, opcode, vchSig);
        pubkey.Set(vchSig.begin(), vchSig.end());
    } else {
        auto &pubkeyWitness = inputView.witnessStack[1];
        pubkey.Set(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length);
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    writer.serialize(*this, scriptNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::MULTISIG>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::MULTISIG> &spendData) : ScriptInputBase(inputView, scriptView) {
    // Prelimary work on code to track multisig spend sets
    
//    CScriptView::const_iterator pc1 = scriptView.begin();
//    opcodetype opcode1;
//    boost::iterator_range<const unsigned char *> vchSig;
//    scriptView.GetOp(pc1, opcode1, vchSig);
//
//    size_t pubkeyNum = 0;
//
//    while(true) {
//        if(!scriptView.GetOp(pc1, opcode1, vchSig)) {
//            break;
//        }
//        if (vchSig.size() > 0) {
//            int hashType = vchSig.back();
//            vchSig.pop_back();
//            auto txHash = tx.getHash(inputView, scriptView, hashType);
//
//            while (pubkeyNum < spendData.addressCount) {
//                std::vector<unsigned char> sig{vchSig.begin(), vchSig.end()};
//                if (spendData.addresses[pubkeyNum].Verify(txHash, sig)) {
//                    spendSet[pubkeyNum] = 1;
//                    pubkeyNum++;
//                    break;
//                } else {
//                    spendSet[pubkeyNum] = 0;
//                    pubkeyNum++;
//                }
//            }
//        }
//    }
//
//    while (pubkeyNum < spendData.addressCount) {
//        spendSet[pubkeyNum] = 0;
//        pubkeyNum++;
//    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::MULTISIG>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    writer.serialize(*this, scriptNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::NONSTANDARD> &) : ScriptInputBase(inputView, scriptView) {
    if (scriptView.size() > 0) {
        script = CScript(scriptView.begin(), scriptView.end());
    } else if (inputView.witnessStack.size() > 0) {
        for (size_t i = 0; i < inputView.witnessStack.size() - 1; i++) {
            auto &stackItem = inputView.witnessStack[i];
            auto itemBegin = reinterpret_cast<const unsigned char *>(stackItem.itemBegin);
            script << std::vector<unsigned char>{itemBegin, itemBegin + stackItem.length};
        }
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    writer.serialize(*this, scriptNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::NULL_DATA> &) : ScriptInputBase(inputView, scriptView) {
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    writer.serialize(*this, scriptNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> &) : ScriptInputBase(inputView, scriptView) {
    auto &pubkeyWitness = inputView.witnessStack[1];
    pubkey.Set(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    writer.serialize(*this, scriptNum);
    return ProcessedInput{};
}

std::pair<AnyScriptOutput, AnyScriptInput> p2shWitnessGenerate(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &) {
    auto &witnessScriptItem = inputView.witnessStack.back();
    auto outputBegin = reinterpret_cast<const unsigned char *>(witnessScriptItem.itemBegin);
    CScriptView witnessView(outputBegin, outputBegin + witnessScriptItem.length);
    auto wrappedScriptOutput = AnyScriptOutput(witnessView, inputView.witnessActivated);
    auto wrappedScriptInput = AnyScriptInput(inputView, scriptView, tx, wrappedScriptOutput);
    return std::make_pair(wrappedScriptOutput, wrappedScriptInput);
}

ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const std::pair<AnyScriptOutput, AnyScriptInput> &data) : ScriptInputBase(inputView, scriptView), wrappedScriptOutput(data.first), wrappedScriptInput(data.second) {
}

ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &spendData) : ScriptInput(inputView, scriptView, p2shWitnessGenerate(inputView, scriptView, tx, spendData)) {}

void ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::process(AddressState &state) {
    wrappedScriptOutput.resolve(state);
    wrappedScriptInput.process(state);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::serialize(uint32_t scriptNum, AddressWriter &writer) {
    bool firstSpend = writer.serialize(*this, scriptNum);
    ProcessedInput processedInput = wrappedScriptInput.serialize(wrappedScriptOutput.address().scriptNum, writer);
    if (firstSpend) {
        processedInput.push_back(scriptNum);
    }
    
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::check(const AddressState &state) {
    wrappedScriptOutput.check(state);
    wrappedScriptInput.check(state);
}
