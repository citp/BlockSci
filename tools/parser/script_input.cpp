//
//  script_input.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_input.hpp"
#include "preproccessed_block.hpp"
#include "output_spend_data.hpp"

#include <secp256k1.h>

#include <iostream>

struct ScriptInputGenerator {
    const InputView &inputView;
    const blocksci::CScriptView &scriptView;
    const RawTransaction &tx;
    ScriptInputGenerator(const InputView &inputView_, const blocksci::CScriptView &scriptView_, const RawTransaction &tx_) : inputView(inputView_), scriptView(scriptView_), tx(tx_) {}
    
    template <blocksci::AddressType::Enum type>
    ScriptInputType operator()(const SpendData<type> &spendData) const {
        return ScriptInput<type>(inputView, scriptView, tx, spendData);
    }
};

AnyScriptInput::AnyScriptInput(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const AnySpendData &spendData) : wrapped(mpark::visit(ScriptInputGenerator(inputView, scriptView, tx), spendData.wrapped)) {}

void AnyScriptInput::process(AddressState &state) {
    mpark::visit([&](auto &scriptInput) { scriptInput.process(state); }, wrapped);
}

void AnyScriptInput::check(AddressState &state) {
    mpark::visit([&](auto &scriptInput) { scriptInput.check(state); }, wrapped);
}

void AnyScriptInput::setScriptNum(uint32_t scriptNum) {
    mpark::visit([&](auto &input) { input.scriptNum = scriptNum; }, wrapped);
}

blocksci::RawAddress AnyScriptInput::address() const {
    return mpark::visit([&](auto &input) { return blocksci::RawAddress{input.scriptNum, input.address_v}; }, wrapped);
}

std::pair<AnyScriptOutput, std::unique_ptr<AnyScriptInput>> p2shGenerate(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::SCRIPTHASH> &) {
    blocksci::CScriptView::const_iterator pc1 = scriptView.begin();
    blocksci::CScriptView::const_iterator prevpc = scriptView.begin();
    blocksci::CScriptView::const_iterator prevprevpc = scriptView.begin();
    blocksci::opcodetype opcode1;
    ranges::iterator_range<const unsigned char *> vch1;
    
    ranges::iterator_range<const unsigned char *> lastScript;
    while(true) {
        prevprevpc = prevpc;
        prevpc = pc1;
        lastScript = vch1;
        if(!scriptView.GetOp(pc1, opcode1, vch1)) {
            break;
        }
    }
    
    blocksci::CScriptView::const_iterator begin = scriptView.begin();
    auto wrappedInputBegin = &*begin;
    auto wrappedInputLength = static_cast<uint32_t>(std::distance(begin, prevprevpc));
    
    blocksci::CScriptView wrappedOutputScript(lastScript.begin(), lastScript.end());
    blocksci::CScriptView p2shScriptView{wrappedInputBegin, wrappedInputBegin + wrappedInputLength};
    AnyScriptOutput wrappedScriptOutput(wrappedOutputScript, false, inputView.witnessActivated);
    return std::make_pair(wrappedScriptOutput, std::make_unique<AnyScriptInput>(inputView, p2shScriptView, tx, AnySpendData{wrappedScriptOutput}));
}

ScriptInputData<blocksci::AddressType::Enum::SCRIPTHASH>::ScriptInputData(std::pair<AnyScriptOutput, std::unique_ptr<AnyScriptInput>> data) : wrappedScriptOutput(data.first), wrappedScriptInput(std::move(data.second)) {}

ScriptInputData<blocksci::AddressType::Enum::SCRIPTHASH>::ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::SCRIPTHASH> &spendData) : ScriptInputData(p2shGenerate(inputView, scriptView, tx, spendData)) {}

void ScriptInputData<blocksci::AddressType::Enum::SCRIPTHASH>::process(AddressState &state) {
    uint32_t scriptNum = wrappedScriptOutput.resolve(state);
    wrappedScriptInput->process(state);
    wrappedScriptInput->setScriptNum(scriptNum);
}

void ScriptInputData<blocksci::AddressType::Enum::SCRIPTHASH>::check(AddressState &state) {
    wrappedScriptOutput.check(state);
    wrappedScriptInput->check(state);
}

ScriptInputData<blocksci::AddressType::Enum::PUBKEYHASH>::ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::PUBKEYHASH> &) {
    pubkey.fill(0);
    if (scriptView.size() > 0) {
        auto pc = scriptView.begin();
        blocksci::opcodetype opcode = blocksci::OP_0;
        ranges::iterator_range<const unsigned char *> vchSig;
        ranges::iterator_range<const unsigned char *> vchSig2;
        // tx 1b008139698117162a9539295ada34fc745f06f733b5f400674f15bf47e720a5 contains a OP_0 before the signature
        // tx bcd1835ebd7e0d44abcab84ec64a488eefd9fa048d2e11a5a24b197838d8af11 (testnet) contains an Push(13) before the real data
        // tx 4c65efdf4e60e9c1bbc1a1a452c3c758789efc7894bff9ed694305eb9c389e7b (testnet) super weird
        
        while (scriptView.GetOp(pc, opcode, vchSig2)) {
            if (vchSig2.size() == 65 || vchSig2.size() == 33) {
                vchSig = vchSig2;
                break;
            }
        }
        assert(vchSig.size() == 65 || vchSig.size() == 33);
        std::copy(vchSig.begin(), vchSig.end(), pubkey.begin());
    } else {
        auto &pubkeyWitness = inputView.witnessStack[1];
        std::copy(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length, pubkey.begin());
    }
}

ScriptInputData<blocksci::AddressType::Enum::MULTISIG>::ScriptInputData(const InputView &, const blocksci::CScriptView &, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::MULTISIG> &) {
    // Prelimary work on code to track multisig spend sets
//    auto multisig = blocksci::CScript() << blocksci::OP_2;
//    for (uint32_t i = 0; i < spendData.addressCount; i++) {
//        std::vector<unsigned char> pubkeyData(spendData.addresses[i].begin(), spendData.addresses[i].end());
//        multisig << pubkeyData;
//    }
//    multisig << blocksci::OP_3 << blocksci::OP_CHECKMULTISIG;
//
//    blocksci::CScriptView::const_iterator pc1 = scriptView.begin();
//    blocksci::opcodetype opcode1;
//    ranges::iterator_range<const unsigned char *> vchSig;
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
//            blocksci::CScriptView multisigView(multisig.data(), multisig.data() + multisig.size());
//            auto txHash = rawTx.getHash(inputView, multisigView, hashType);
//
//            while (pubkeyNum < spendData.addressCount) {
//                std::vector<unsigned char> sig{vchSig.begin(), vchSig.end() - 1};
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

ScriptInputData<blocksci::AddressType::Enum::NONSTANDARD>::ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::NONSTANDARD> &) {
    if (scriptView.size() > 0) {
        script = blocksci::CScript(scriptView.begin(), scriptView.end());
    } else if (inputView.witnessStack.size() > 0) {
        for (size_t i = 0; i < inputView.witnessStack.size() - 1; i++) {
            auto &stackItem = inputView.witnessStack[i];
            auto itemBegin = reinterpret_cast<const unsigned char *>(stackItem.itemBegin);
            script << std::vector<unsigned char>{itemBegin, itemBegin + stackItem.length};
        }
    }
}

ScriptInputData<blocksci::AddressType::Enum::NULL_DATA>::ScriptInputData(const InputView &, const blocksci::CScriptView &, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::NULL_DATA> &) {
}

ScriptInputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::ScriptInputData(const InputView &inputView, const blocksci::CScriptView &, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> &) {
    pubkey.fill(0);
    auto &pubkeyWitness = inputView.witnessStack[1];
    std::copy(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length, pubkey.begin());
}

std::pair<AnyScriptOutput, std::unique_ptr<AnyScriptInput>> p2shWitnessGenerate(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &) {
    assert(inputView.witnessStack.size() > 0);
    auto &witnessScriptItem = inputView.witnessStack.back();
    auto outputBegin = reinterpret_cast<const unsigned char *>(witnessScriptItem.itemBegin);
    blocksci::CScriptView witnessView(outputBegin, outputBegin + witnessScriptItem.length);
    AnyScriptOutput wrappedScriptOutput(witnessView, false, inputView.witnessActivated);
    return std::make_pair(wrappedScriptOutput, std::make_unique<AnyScriptInput>(inputView, scriptView, tx, AnySpendData{wrappedScriptOutput}));
}

ScriptInputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::ScriptInputData(std::pair<AnyScriptOutput, std::unique_ptr<AnyScriptInput>> data) : wrappedScriptOutput(data.first), wrappedScriptInput(std::move(data.second)) {
}

ScriptInputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &spendData) : ScriptInputData(p2shWitnessGenerate(inputView, scriptView, tx, spendData)) {}

void ScriptInputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::process(AddressState &state) {
    uint32_t scriptNum = wrappedScriptOutput.resolve(state);
    wrappedScriptInput->process(state);
    wrappedScriptInput->setScriptNum(scriptNum);
}

void ScriptInputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::check(AddressState &state) {
    wrappedScriptOutput.check(state);
    wrappedScriptInput->check(state);
}

ScriptInputData<blocksci::AddressType::Enum::WITNESS_UNKNOWN>::ScriptInputData(const InputView &inputView, const blocksci::CScriptView &, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::WITNESS_UNKNOWN> &) {
    for (size_t i = 0; i < inputView.witnessStack.size(); i++) {
        auto &stackItem = inputView.witnessStack[i];
        auto itemBegin = reinterpret_cast<const unsigned char *>(stackItem.itemBegin);
        script << std::vector<unsigned char>{itemBegin, itemBegin + stackItem.length};
        script << 0xfe;
    }
}
