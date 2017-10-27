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
#include <blocksci/scripts/bitcoin_pubkey.hpp>

#include <secp256k1.h>

#include <iostream>


template<blocksci::AddressType::Enum type>
struct ProcessScriptInputFunctor {
    static ProcessedInput f(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter) {
        auto input = ScriptInput<type>(inputView, scriptView, tx, addressWriter);
        return input.processInput(inputView, scriptView, tx, state, addressWriter);
    }
};

ProcessedInput processInput(const blocksci::Address &address, const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter) {
    
    static constexpr auto table = blocksci::make_dynamic_table<blocksci::AddressType, ProcessScriptInputFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(address.type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](inputView, scriptView, tx, state, addressWriter);
}

template<blocksci::AddressType::Enum type>
struct CheckScriptInputFunctor {
    static void f(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &addressWriter) {
        ScriptInput<type> input(inputView, scriptView, tx, addressWriter);
        input.checkInput(inputView, scriptView, tx, state, addressWriter);
    }
};

void checkInput(blocksci::AddressType::Enum type, const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &addressWriter) {
    static constexpr auto table = blocksci::make_dynamic_table<blocksci::AddressType, CheckScriptInputFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    table[index](inputView, scriptView, tx, state, addressWriter);
}

ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, const AddressWriter &) {
    CScriptView script(scriptView.getScript());
    
    CScriptView::const_iterator pc1 = script.begin();
    CScriptView::const_iterator prevpc = script.begin();
    CScriptView::const_iterator prevprevpc = script.begin();
    opcodetype opcode1;
    boost::iterator_range<const unsigned char *> vch1;
    
    boost::iterator_range<const unsigned char *> lastScript;
    while(true) {
        prevprevpc = prevpc;
        prevpc = pc1;
        lastScript = vch1;
        if(!script.GetOp(pc1, opcode1, vch1)) {
            break;
        }
    }
    
    CScriptView::const_iterator begin = script.begin();
    wrappedInputBegin = &*begin;
    wrappedInputLength = static_cast<uint32_t>(std::distance(begin, prevprevpc));
    
    CScriptView wrappedOutputScript(lastScript.begin(), lastScript.end());
    wrappedScriptOutput = extractScriptData(lastScript.begin(), lastScript.end(), inputView.witnessActivated);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &writer) {
    auto [address, isNew] = resolveAddress(wrappedScriptOutput, state);
    if (isNew) {
        writer.serialize(wrappedScriptOutput);
    }
    wrappedAddress = address;
    
    InputScriptView p2shScriptView{wrappedAddress.scriptNum, wrappedInputBegin, wrappedInputLength};
    ProcessedInput processedInput = ::processInput(wrappedAddress, inputView, p2shScriptView, tx, state, writer);
    bool firstSpend = writer.serialize(*this, scriptView.scriptNum, inputView.txNum);
    if (firstSpend) {
        processedInput.push_back(scriptView.scriptNum);
    }
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::checkInput(const InputView &inputView, const InputScriptView &, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer) {
    wrappedAddress = checkOutput(wrappedScriptOutput, state, writer);
    InputScriptView p2shScriptView{wrappedAddress.scriptNum, wrappedInputBegin, wrappedInputLength};
    ::checkInput(wrappedAddress.type, inputView, p2shScriptView, tx, state, writer);
}

ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, const AddressWriter &) {
    if (scriptView.scriptLength > 0) {
        auto script = scriptView.getScript();
        auto pc = script.begin();
        opcodetype opcode;
        boost::iterator_range<const unsigned char *> vchSig;
        script.GetOp(pc, opcode, vchSig);
        script.GetOp(pc, opcode, vchSig);
        pubkey.Set(vchSig.begin(), vchSig.end());
    } else {
        auto &pubkeyWitness = inputView.witnessStack[1];
        pubkey.Set(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length);
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, scriptView.scriptNum, inputView.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::MULTISIG>::ScriptInput(const InputView &, const InputScriptView &, const RawTransaction &, const AddressWriter &) {
    // Prelimary work on code to track multisig spend sets
    /*
    auto &multisigFile = writer.getFile<blocksci::AddressType::Enum::MULTISIG>();
    auto &pubkeyFile = writer.getFile<blocksci::AddressType::Enum::PUBKEY>();
    auto multisigData = multisigFile.getData(inputInfo.address.addressNum - 1);
    auto addresses = multisigData->getAddresses();
    
    auto script = inputInfo.getScript();
    
    CScript::const_iterator pc1 = script.begin();
    opcodetype opcode1;
    std::vector<unsigned char> vchSig;
    script.GetOp(pc1, opcode1, vchSig);
    
    size_t pubkeyNum = 0;
    
    while(true) {
        if(!script.GetOp(pc1, opcode1, vchSig)) {
            break;
        }
        if (vchSig.size() > 0) {
            int hashType = vchSig.back();
            vchSig.pop_back();
            auto txHash = tx.getHash(inputInfo, hashType);
            
            while (pubkeyNum < addresses.size()) {
                auto pubkeyAddress = pubkeyFile.getData(addresses[pubkeyNum].addressNum - 1);
                if (pubkeyAddress->pubkey.Verify(txHash, vchSig)) {
                    spendSet[pubkeyNum] = 1;
                    pubkeyNum++;
                    break;
                } else {
                    spendSet[pubkeyNum] = 0;
                    pubkeyNum++;
                }
            }
        }
    }
    
    while (pubkeyNum < addresses.size()) {
        spendSet[pubkeyNum] = 0;
        pubkeyNum++;
    }
    */
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::MULTISIG>::processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, scriptView.scriptNum, inputView.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, const AddressWriter &) {
    if (scriptView.scriptLength > 0) {
        auto scriptViewer = scriptView.getScript();
        script = CScript(scriptViewer.begin(), scriptViewer.end());
    } else if (inputView.witnessStack.size() > 0) {
        for (size_t i = 0; i < inputView.witnessStack.size() - 1; i++) {
            auto &stackItem = inputView.witnessStack[i];
            auto itemBegin = reinterpret_cast<const unsigned char *>(stackItem.itemBegin);
            script << std::vector<unsigned char>{itemBegin, itemBegin + stackItem.length};
        }
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, scriptView.scriptNum, inputView.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::ScriptInput(const InputView &, const InputScriptView &, const RawTransaction &, const AddressWriter &) {
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, scriptView.scriptNum, inputView.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::ScriptInput(const InputView &inputView, const InputScriptView &, const RawTransaction &, const AddressWriter &) {
    auto &pubkeyWitness = inputView.witnessStack[1];
    pubkey.Set(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, scriptView.scriptNum, inputView.txNum);
    return ProcessedInput{};
}


ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::ScriptInput(const InputView &inputView, const InputScriptView &, const RawTransaction &, const AddressWriter &) {
    auto &witnessScriptItem = inputView.witnessStack.back();
    auto outputBegin = reinterpret_cast<const unsigned char *>(witnessScriptItem.itemBegin);
    wrappedScriptOutput = extractScriptData(outputBegin, outputBegin + witnessScriptItem.length, inputView.witnessActivated);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &writer) {
    
    auto [address, isNew] = resolveAddress(wrappedScriptOutput, state);
    if (isNew) {
        writer.serialize(wrappedScriptOutput);
    }
    wrappedAddress = address;

    InputScriptView p2shScriptView{wrappedAddress.scriptNum, scriptView.scriptBegin, 0};
    auto processedInput = ::processInput(wrappedAddress, inputView, p2shScriptView, tx, state, writer);
    
    bool firstSpend = writer.serialize(*this, scriptView.scriptNum, inputView.txNum);
    if (firstSpend) {
        processedInput.push_back(scriptView.scriptNum);
    }
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::checkInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer) {
    wrappedAddress = checkOutput(wrappedScriptOutput, state, writer);
    InputScriptView p2shScriptView{wrappedAddress.scriptNum, scriptView.scriptBegin, 0};
    ::checkInput(wrappedAddress.type, inputView, p2shScriptView, tx, state, writer);
}
