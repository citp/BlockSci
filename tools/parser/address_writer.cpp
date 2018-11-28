//
//  address_writer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_writer.hpp"
#include "preproccessed_block.hpp"

using blocksci::AddressType;
using blocksci::DedupAddressType;

AddressWriter::AddressWriter(const ParserConfigurationBase &config) :
scriptFiles(blocksci::apply(blocksci::DedupAddressType::all(), [&] (auto tag) {
    return (filesystem::path{config.dataConfig.scriptsDirectory()}/std::string{dedupAddressName(tag)}).str();
})) {
}

blocksci::OffsetType AddressWriter::serialize(const AnyScriptOutput &output, uint32_t txNum, bool topLevel) {
    return mpark::visit([&](auto &scriptOutput) { return this->serialize(scriptOutput, txNum, topLevel); }, output.wrapped);
}

void AddressWriter::serialize(const AnyScriptInput &input, uint32_t txNum, uint32_t outputTxNum) {
    mpark::visit([&](auto &scriptInput) { this->serialize(scriptInput, txNum, outputTxNum); }, input.wrapped);
}

void AddressWriter::serializeWrapped(const AnyScriptInput &input, uint32_t txNum, uint32_t outputTxNum) {
    mpark::visit([&](auto &scriptInput) { this->serializeWrapped(scriptInput.data, txNum, outputTxNum); }, input.wrapped);
}

void AddressWriter::serializeImp(const ScriptOutput<AddressType::PUBKEY> &output, ScriptFile<DedupAddressType::PUBKEY> &file, bool topLevel) {
    auto data = file[output.scriptNum - 1];
    std::copy(output.data.pubkey.begin(), output.data.pubkey.end(), data->pubkey.begin());
    data->hasPubkey = true;
    data->saw(AddressType::PUBKEY, topLevel);
}

void AddressWriter::serializeImp(const ScriptOutput<AddressType::MULTISIG_PUBKEY> &output, ScriptFile<DedupAddressType::PUBKEY> &file, bool topLevel) {
    auto data = file[output.scriptNum - 1];
    std::copy(output.data.pubkey.begin(), output.data.pubkey.end(), data->pubkey.begin());
    data->saw(AddressType::MULTISIG_PUBKEY, topLevel);
    data->hasPubkey = true;
}

void AddressWriter::serializeImp(const ScriptOutput<AddressType::WITNESS_SCRIPTHASH> &output, ScriptFile<blocksci::DedupAddressType::SCRIPTHASH> &file, bool topLevel) {
    auto data = file[output.scriptNum - 1];
    data->hash256 = output.data.hash;
    data->isSegwit = true;
    data->saw(AddressType::WITNESS_SCRIPTHASH, topLevel);
}

void AddressWriter::serializeImp(const ScriptOutput<AddressType::NONSTANDARD> &output, ScriptFile<blocksci::DedupAddressType::NONSTANDARD> &file, bool topLevel) {
    auto data = file[output.scriptNum - 1];
    std::get<0>(data)->saw(AddressType::NONSTANDARD, topLevel);
}
void AddressWriter::serializeImp(const ScriptOutput<AddressType::WITNESS_UNKNOWN> &output, ScriptFile<blocksci::DedupAddressType::WITNESS_UNKNOWN> &file, bool topLevel) {
    auto data = file[output.scriptNum - 1];
    std::get<0>(data)->saw(AddressType::WITNESS_UNKNOWN, topLevel);
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::PUBKEYHASH> &input, ScriptFile<DedupAddressType::PUBKEY> &file) {
    auto data = file[input.scriptNum - 1];
    if (!data->hasPubkey) {
        std::copy(input.data.pubkey.begin(), input.data.pubkey.end(), data->pubkey.begin());
        data->hasPubkey = true;
    }
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::WITNESS_PUBKEYHASH> &input, ScriptFile<DedupAddressType::PUBKEY> &file) {
    auto data = file[input.scriptNum - 1];
    if (!data->hasPubkey) {
        std::copy(input.data.pubkey.begin(), input.data.pubkey.end(), data->pubkey.begin());
        data->hasPubkey = true;
    }
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::WITNESS_SCRIPTHASH> &input, ScriptFile<DedupAddressType::SCRIPTHASH> &file) {
    auto data = file[input.scriptNum - 1];
    data->wrappedAddress = input.data.wrappedScriptOutput.address();
}

void AddressWriter::serializeWrapped(const ScriptInputData<AddressType::Enum::WITNESS_SCRIPTHASH> &data, uint32_t txNum, uint32_t outputTxNum) {
    serialize(data.wrappedScriptOutput, txNum, false);
    serialize(*data.wrappedScriptInput, txNum, outputTxNum);
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::SCRIPTHASH> &input, ScriptFile<DedupAddressType::SCRIPTHASH> &file) {
    auto data = file[input.scriptNum - 1];
    data->wrappedAddress = input.data.wrappedScriptOutput.address();
}

void AddressWriter::serializeWrapped(const ScriptInputData<AddressType::Enum::SCRIPTHASH> &data, uint32_t txNum, uint32_t outputTxNum) {
    serialize(data.wrappedScriptOutput, txNum, false);
    return serialize(*data.wrappedScriptInput, txNum, outputTxNum);
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::NONSTANDARD> &input, ScriptFile<DedupAddressType::NONSTANDARD> &file) {
    blocksci::NonstandardSpendScriptData scriptData(static_cast<uint32_t>(input.data.script.size()));
    blocksci::ArbitraryLengthData<blocksci::NonstandardSpendScriptData> data(scriptData);
    data.add(input.data.script.begin(), input.data.script.end());
    file.write<1>(input.scriptNum - 1, data);
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::WITNESS_UNKNOWN> &input, ScriptFile<DedupAddressType::WITNESS_UNKNOWN> &file) {
    blocksci::WitnessUnknownSpendScriptData scriptData(static_cast<uint32_t>(input.data.script.size()));
    blocksci::ArbitraryLengthData<blocksci::WitnessUnknownSpendScriptData> data(scriptData);
    data.add(input.data.script.begin(), input.data.script.end());
    file.write<1>(input.scriptNum - 1, data);
}

void AddressWriter::rollback(const blocksci::State &state) {
    blocksci::for_each(blocksci::DedupAddressType::all(), [&](auto tag) {
        auto &file = std::get<ScriptFile<tag()>>(scriptFiles);
        file.truncate(state.scriptCounts[static_cast<size_t>(tag)]);
    });
}
