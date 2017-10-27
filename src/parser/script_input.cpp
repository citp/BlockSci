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
    static ProcessedInput f(const InputInfo &info, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter) {
        auto input = ScriptInput<type>(info, tx, addressWriter);
        return input.processInput(info, tx, state, addressWriter);
    }
};

ProcessedInput processInput(const blocksci::Address &address, const InputInfo &info, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter) {
    
    static constexpr auto table = blocksci::make_dynamic_table<blocksci::AddressType, ProcessScriptInputFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(address.type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](info, tx, state, addressWriter);
}

template<blocksci::AddressType::Enum type>
struct CheckScriptInputFunctor {
    static void f(const InputInfo &info, const RawTransaction &tx, const AddressState &state, const AddressWriter &addressWriter) {
        ScriptInput<type> input(info, tx, addressWriter);
        input.checkInput(info, tx, state, addressWriter);
    }
};

void checkInput(blocksci::AddressType::Enum type, const InputInfo &info, const RawTransaction &tx, const AddressState &state, const AddressWriter &addressWriter) {
    static constexpr auto table = blocksci::make_dynamic_table<blocksci::AddressType, CheckScriptInputFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    table[index](info, tx, state, addressWriter);
}

ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    CScriptView script(inputInfo.getScript());
    
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
    wrappedScriptOutput = extractScriptData(lastScript.begin(), lastScript.end(), inputInfo.witnessActivated);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &tx, AddressState &state, AddressWriter &writer) {
    auto [address, isNew] = resolveAddress(wrappedScriptOutput, state);
    if (isNew) {
        writer.serialize(wrappedScriptOutput);
    }
    wrappedAddress = address;
    
    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.scriptNum, wrappedInputBegin, wrappedInputLength, inputInfo.witnessStack, inputInfo.witnessActivated};
    ProcessedInput processedInput = ::processInput(wrappedAddress, p2shInputInfo, tx, state, writer);
    bool firstSpend = writer.serialize(*this, inputInfo.addressNum, inputInfo.txNum);
    if (firstSpend) {
        processedInput.push_back(inputInfo.addressNum);
    }
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer) {
    wrappedAddress = checkOutput(wrappedScriptOutput, state, writer);
    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.scriptNum, wrappedInputBegin, wrappedInputLength, inputInfo.witnessStack, inputInfo.witnessActivated};
    ::checkInput(wrappedAddress.type, p2shInputInfo, tx, state, writer);
}

ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    if (inputInfo.scriptLength > 0) {
        auto script = inputInfo.getScript();
        auto pc = script.begin();
        opcodetype opcode;
        boost::iterator_range<const unsigned char *> vchSig;
        script.GetOp(pc, opcode, vchSig);
        script.GetOp(pc, opcode, vchSig);
        pubkey.Set(vchSig.begin(), vchSig.end());
    } else {
        auto &pubkeyWitness = inputInfo.witnessStack[1];
        pubkey.Set(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length);
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo.addressNum, inputInfo.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::MULTISIG>::ScriptInput(const InputInfo &, const RawTransaction &, const AddressWriter &) {
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

ProcessedInput ScriptInput<blocksci::AddressType::Enum::MULTISIG>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo.addressNum, inputInfo.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    if (inputInfo.scriptLength > 0) {
        auto scriptView = inputInfo.getScript();
        script = CScript(scriptView.begin(), scriptView.end());
    } else if (inputInfo.witnessStack.size() > 0) {
        for (size_t i = 0; i < inputInfo.witnessStack.size() - 1; i++) {
            auto &stackItem = inputInfo.witnessStack[i];
            auto itemBegin = reinterpret_cast<const unsigned char *>(stackItem.itemBegin);
            script << std::vector<unsigned char>{itemBegin, itemBegin + stackItem.length};
        }
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo.addressNum, inputInfo.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::ScriptInput(const InputInfo &, const RawTransaction &, const AddressWriter &) {
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo.addressNum, inputInfo.txNum);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    auto &pubkeyWitness = inputInfo.witnessStack[1];
    pubkey.Set(reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin), reinterpret_cast<const unsigned char *>(pubkeyWitness.itemBegin) + pubkeyWitness.length);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo.addressNum, inputInfo.txNum);
    return ProcessedInput{};
}


ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    auto &witnessScriptItem = inputInfo.witnessStack.back();
    auto outputBegin = reinterpret_cast<const unsigned char *>(witnessScriptItem.itemBegin);
    wrappedScriptOutput = extractScriptData(outputBegin, outputBegin + witnessScriptItem.length, inputInfo.witnessActivated);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &tx, AddressState &state, AddressWriter &writer) {
    
    auto [address, isNew] = resolveAddress(wrappedScriptOutput, state);
    if (isNew) {
        writer.serialize(wrappedScriptOutput);
    }
    wrappedAddress = address;

    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.scriptNum, inputInfo.scriptBegin, 0, inputInfo.witnessStack, inputInfo.witnessActivated};
    auto processedInput = ::processInput(wrappedAddress, p2shInputInfo, tx, state, writer);
    
    bool firstSpend = writer.serialize(*this, inputInfo.addressNum, inputInfo.txNum);
    if (firstSpend) {
        processedInput.push_back(inputInfo.addressNum);
    }
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer) {
    wrappedAddress = checkOutput(wrappedScriptOutput, state, writer);
    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.scriptNum, inputInfo.scriptBegin, 0, inputInfo.witnessStack, inputInfo.witnessActivated};
    ::checkInput(wrappedAddress.type, p2shInputInfo, tx, state, writer);
}
