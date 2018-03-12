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
scriptFiles(blocksci::apply(blocksci::DedupAddressInfoList(), [&] (auto tag) {
    return ScriptFile<tag>(config.scriptsDirectory()/ std::string{equivAddressName(tag)});
})) {
}

size_t AddressWriter::serialize(const AnyScriptOutput &output, uint32_t txNum) {
    return mpark::visit([&](auto &scriptOutput) { return this->serialize(scriptOutput, txNum); }, output.wrapped);
}

void AddressWriter::serialize(const AnyScriptInput &input, uint32_t txNum, uint32_t outputTxNum) {
    mpark::visit([&](auto &scriptInput) { this->serialize(scriptInput, txNum, outputTxNum); }, input.wrapped);
}

using namespace blocksci;

void AddressWriter::serializeImp(const ScriptOutput<blocksci::AddressType::PUBKEY> &output, ScriptFile<blocksci::DedupAddressType::PUBKEY> &file) {
    auto data = file.getData(output.scriptNum - 1);
    data->pubkey = output.data.pubkey;
}

void AddressWriter::serializeImp(const ScriptOutput<blocksci::AddressType::WITNESS_SCRIPTHASH> &output, ScriptFile<blocksci::DedupAddressType::SCRIPTHASH> &file) {
    auto data = file.getData(output.scriptNum - 1);
    data->hash256 = output.data.hash;
    data->isSegwit = true;
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::PUBKEYHASH> &input, ScriptFile<DedupAddressType::PUBKEY> &file) {
    auto data = file.getData(input.scriptNum - 1);
    data->pubkey = input.data.pubkey;
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::WITNESS_PUBKEYHASH> &input, ScriptFile<DedupAddressType::PUBKEY> &file) {
    auto data = file.getData(input.scriptNum - 1);
    data->pubkey = input.data.pubkey;
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::WITNESS_SCRIPTHASH> &input, ScriptFile<DedupAddressType::SCRIPTHASH> &file) {
    auto data = file.getData(input.scriptNum - 1);
    data->wrappedAddress = input.data.wrappedScriptOutput.address();
}

void AddressWriter::serializeWrapped(const ScriptInputData<AddressType::Enum::WITNESS_SCRIPTHASH> &data, uint32_t txNum, uint32_t outputTxNum) {
    serialize(data.wrappedScriptOutput, txNum);
    serialize(*data.wrappedScriptInput, txNum, outputTxNum);
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::SCRIPTHASH> &input, ScriptFile<DedupAddressType::SCRIPTHASH> &file) {
    auto data = file.getData(input.scriptNum - 1);
    data->wrappedAddress = input.data.wrappedScriptOutput.address();
}

void AddressWriter::serializeWrapped(const ScriptInputData<AddressType::Enum::SCRIPTHASH> &data, uint32_t txNum, uint32_t outputTxNum) {
    serialize(data.wrappedScriptOutput, txNum);
    return serialize(*data.wrappedScriptInput, txNum, outputTxNum);
}

void AddressWriter::serializeImp(const ScriptInput<AddressType::NONSTANDARD> &input, ScriptFile<DedupAddressType::NONSTANDARD> &file) {
    NonstandardSpendScriptData scriptData(static_cast<uint32_t>(input.data.script.size()));
    blocksci::ArbitraryLengthData<NonstandardSpendScriptData> data(scriptData);
    data.add(input.data.script.begin(), input.data.script.end());
    file.write<1>(input.scriptNum - 1, data);
}

void AddressWriter::rollback(const blocksci::State &state) {
    blocksci::for_each(blocksci::DedupAddressInfoList(), [&](auto tag) {
        auto &file = std::get<ScriptFile<tag()>>(scriptFiles);
        file.truncate(state.scriptCounts[static_cast<size_t>(tag)]);
    });
}
