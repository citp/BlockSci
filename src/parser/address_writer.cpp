//
//  address_writer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_writer.hpp"
#include <blocksci/scripts/script.hpp>

AddressWriter::AddressWriter(const ParserConfigurationBase &config) :
scriptFiles(blocksci::apply(blocksci::ScriptInfoList(), [&] (auto tag) {
    return config.scriptsDirectory()/ std::string{scriptName(tag)};
})) {
}

uint32_t AddressWriter::serialize(const AnyScriptOutput &output) {
    return boost::apply_visitor([&](auto &scriptOutput) { return serialize(scriptOutput); }, output.wrapped);
}

ProcessedInput AddressWriter::serialize(const AnyScriptInput &input) {
    return boost::apply_visitor([&](auto &scriptInput) { return serialize(scriptInput); }, input.wrapped);
}

using namespace blocksci;

constexpr static CPubKey nullPubkey{};

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::PUBKEYHASH> &input, ScriptFile<ScriptType::Enum::PUBKEY> &file) {
    auto data = file.getData(input.scriptNum - 1);
    if (data->pubkey == nullPubkey) {
        data->pubkey = input.data.pubkey;
        return ProcessedInput{Script(input.scriptNum, ScriptType::Enum::PUBKEY)};
    }
    return ProcessedInput{};
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::WITNESS_PUBKEYHASH> &input, ScriptFile<ScriptType::Enum::PUBKEY> &file) {
    auto data = file.getData(input.scriptNum - 1);
    if (data->pubkey == nullPubkey) {
        data->pubkey = input.data.pubkey;
        return ProcessedInput{Script(input.scriptNum, ScriptType::Enum::PUBKEY)};
    }
    return ProcessedInput{};
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::WITNESS_SCRIPTHASH> &input, ScriptFile<ScriptType::Enum::SCRIPTHASH> &file) {
    serialize(input.data.wrappedScriptOutput);
    auto processed = serialize(input.data.wrappedScriptInput);
    auto data = file.getData(input.scriptNum - 1);
    if (data->wrappedAddress.scriptNum == 0) {
        data->wrappedAddress = input.data.wrappedScriptOutput.address();
        data->txRevealed = input.data.txNum;
        processed.emplace_back(input.scriptNum, ScriptType::Enum::SCRIPTHASH);
    }
    return processed;
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::SCRIPTHASH> &input, ScriptFile<ScriptType::Enum::SCRIPTHASH> &file) {
    serialize(input.data.wrappedScriptOutput);
    auto processed = serialize(input.data.wrappedScriptInput);
    auto data = file.getData(input.scriptNum - 1);
    if (data->wrappedAddress.scriptNum == 0) {
        data->wrappedAddress = input.data.wrappedScriptOutput.address();
        data->txRevealed = input.data.txNum;
        processed.emplace_back(input.scriptNum, ScriptType::Enum::SCRIPTHASH);
    }
    return processed;
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::NONSTANDARD> &input, ScriptFile<ScriptType::Enum::NONSTANDARD> &file) {
    NonstandardScriptData scriptData(input.data.script);
    blocksci::ArbitraryLengthData<NonstandardScriptData> data(scriptData);
    data.add(input.data.script.begin(), input.data.script.end());
    file.write<1>(input.scriptNum - 1, data);
    return ProcessedInput{Script(input.scriptNum, ScriptType::Enum::NONSTANDARD)};
}

void AddressWriter::rollback(const blocksci::State &state) {
    blocksci::for_each(blocksci::ScriptInfoList(), [&](auto tag) {
        auto &file = std::get<ScriptFile<tag()>>(scriptFiles);
        file.truncate(state.scriptCounts[static_cast<size_t>(tag)]);
    });
}
