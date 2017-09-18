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
    CScript script(inputInfo.getScript());
    
    CScript::const_iterator pc1 = script.begin();
    CScript::const_iterator prevpc = script.begin();
    CScript::const_iterator prevprevpc = script.begin();
    opcodetype opcode1;
    std::vector<unsigned char> vch1;
    
    std::vector<unsigned char> lastScript;
    while(true) {
        prevprevpc = prevpc;
        prevpc = pc1;
        lastScript = vch1;
        if(!script.GetOp(pc1, opcode1, vch1)) {
            break;
        }
    }
    
    CScript::const_iterator begin = script.begin();
    wrappedInputBegin = &*begin;
    wrappedInputLength = std::distance(begin, prevprevpc);
    
    CScript wrappedOutputScript(lastScript.begin(), lastScript.end());
    auto outputScriptBegin = reinterpret_cast<const unsigned char *>(lastScript.data());
    wrappedScriptOutput = extractScriptData(outputScriptBegin, outputScriptBegin + lastScript.size(), inputInfo.witnessActivated);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &tx, AddressState &state, AddressWriter &writer) {
    wrappedAddress = processOutput(wrappedScriptOutput, state, writer);
    
    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.addressNum, wrappedInputBegin, wrappedInputLength, inputInfo.witnessStack, inputInfo.witnessActivated};
    ProcessedInput processedInput = ::processInput(wrappedAddress, p2shInputInfo, tx, state, writer);
    bool firstSpend = writer.serialize(*this, inputInfo);
    if (firstSpend) {
        processedInput.push_back(inputInfo.addressNum);
    }
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>::checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer) {
    wrappedAddress = checkOutput(wrappedScriptOutput, state);
    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.addressNum, wrappedInputBegin, wrappedInputLength, inputInfo.witnessStack, inputInfo.witnessActivated};
    ::checkInput(wrappedAddress.type, p2shInputInfo, tx, state, writer);
}

ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    if (inputInfo.scriptLength > 0) {
        auto script = inputInfo.getScript();
        auto pc = script.begin();
        opcodetype opcode;
        std::vector<unsigned char> vchSig;
        script.GetOp(pc, opcode, vchSig);
        script.GetOp(pc, opcode, vchSig);
        pubkey.Set(vchSig.begin(), vchSig.end());
    } else {
        auto &pubkeyWitness = inputInfo.witnessStack[1];
        pubkey.Set(pubkeyWitness.itemBegin, pubkeyWitness.itemBegin + pubkeyWitness.length);
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo);
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
    writer.serialize(*this, inputInfo);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    if (inputInfo.scriptLength > 0) {
        script = inputInfo.getScript();
    } else if (inputInfo.witnessStack.size() > 0) {
        for (size_t i = 0; i < inputInfo.witnessStack.size() - 1; i++) {
            auto &stackItem = inputInfo.witnessStack[i];
            auto itemBegin = reinterpret_cast<const unsigned char *>(stackItem.itemBegin);
            script << std::vector<unsigned char>{itemBegin, itemBegin + stackItem.length};
        }
    }
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NONSTANDARD>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::ScriptInput(const InputInfo &, const RawTransaction &, const AddressWriter &) {
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::NULL_DATA>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo);
    return ProcessedInput{};
}

ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    auto &pubkeyWitness = inputInfo.witnessStack[1];
    pubkey.Set(pubkeyWitness.itemBegin, pubkeyWitness.itemBegin + pubkeyWitness.length);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &, AddressState &, AddressWriter &writer) {
    writer.serialize(*this, inputInfo);
    return ProcessedInput{};
}


ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::ScriptInput(const InputInfo &inputInfo, const RawTransaction &, const AddressWriter &) {
    auto &witnessScriptItem = inputInfo.witnessStack.back();
    auto outputBegin = reinterpret_cast<const unsigned char *>(witnessScriptItem.itemBegin);
    wrappedScriptOutput = extractScriptData(outputBegin, outputBegin + witnessScriptItem.length, inputInfo.witnessActivated);
}

ProcessedInput ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::processInput(const InputInfo &inputInfo, const RawTransaction &tx, AddressState &state, AddressWriter &writer) {
    
    wrappedAddress = processOutput(wrappedScriptOutput, state, writer);

    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.addressNum, inputInfo.scriptBegin, 0, inputInfo.witnessStack, inputInfo.witnessActivated};
    auto processedInput = ::processInput(wrappedAddress, p2shInputInfo, tx, state, writer);
    
    bool firstSpend = writer.serialize(*this, inputInfo);
    if (firstSpend) {
        processedInput.push_back(inputInfo.addressNum);
    }
    return processedInput;
}

void ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer) {
    wrappedAddress = checkOutput(wrappedScriptOutput, state);
    InputInfo p2shInputInfo{inputInfo.inputNum, inputInfo.txNum, wrappedAddress.addressNum, inputInfo.scriptBegin, 0, inputInfo.witnessStack, inputInfo.witnessActivated};
    ::checkInput(wrappedAddress.type, p2shInputInfo, tx, state, writer);
}
