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

struct SerializeOutputVisitor : public boost::static_visitor<uint32_t> {
    AddressWriter &writer;
    
    SerializeOutputVisitor(AddressWriter &writer_) : writer(writer_) {}
    template <blocksci::AddressType::Enum type>
    uint32_t operator()(const ScriptOutput<type> &data) const
    {
        return writer.serialize(data);
    }
};

struct SerializeInputVisitor : public boost::static_visitor<ProcessedInput> {
    AddressWriter &writer;
    
    SerializeInputVisitor(AddressWriter &writer_) : writer(writer_) {}
    template <blocksci::AddressType::Enum type>
    ProcessedInput operator()(const ScriptInput<type> &data) const {
        return writer.serialize(data);
    }
};

uint32_t AddressWriter::serialize(const AnyScriptOutput &output) {
    SerializeOutputVisitor visitor(*this);
    return boost::apply_visitor(visitor, output.wrapped);
}

ProcessedInput AddressWriter::serialize(const AnyScriptInput &input) {
    SerializeInputVisitor visitor(*this);
    return boost::apply_visitor(visitor, input.wrapped);
}

using namespace blocksci;

void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::PUBKEY> &output, ScriptFile<ScriptType::Enum::PUBKEY> &file) {
    file.write({output.pubkey, output.pubkey.GetID()});
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::PUBKEY> &, ScriptFile<ScriptType::Enum::PUBKEY> &) {
    return ProcessedInput{};
}

constexpr static CPubKey nullPubkey{};

void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::PUBKEYHASH> &output, ScriptFile<ScriptType::Enum::PUBKEY> &file) {
    file.write({nullPubkey, output.hash});
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::PUBKEYHASH> &input, ScriptFile<ScriptType::Enum::PUBKEY> &file) {
    auto data = file.getData(input.scriptNum - 1);
    if (data->pubkey == nullPubkey) {
        data->pubkey = input.data.pubkey;
        return ProcessedInput{Script(input.scriptNum, ScriptType::Enum::PUBKEY)};
    }
    return ProcessedInput{};
}


void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::WITNESS_PUBKEYHASH> &output, ScriptFile<ScriptType::Enum::PUBKEY> &file) {
    file.write({nullPubkey, output.hash});
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::WITNESS_PUBKEYHASH> &input, ScriptFile<ScriptType::Enum::PUBKEY> &file) {
    auto data = file.getData(input.scriptNum - 1);
    if (data->pubkey == nullPubkey) {
        data->pubkey = input.data.pubkey;
        return ProcessedInput{Script(input.scriptNum, ScriptType::Enum::PUBKEY)};
    }
    return ProcessedInput{};
}

void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::WITNESS_SCRIPTHASH> &output, ScriptFile<ScriptType::Enum::SCRIPTHASH> &file) {
    blocksci::Address wrappedAddress;
    file.write({output.getHash(), wrappedAddress, 0});
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

void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::SCRIPTHASH> &output, ScriptFile<ScriptType::Enum::SCRIPTHASH> &file) {
    blocksci::Address wrappedAddress;
    file.write({output.hash, wrappedAddress, 0});
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

void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::MULTISIG> &output, ScriptFile<ScriptType::Enum::MULTISIG> &file) {
    MultisigData multisigData{output.numRequired, output.numTotal, output.addressCount};
    blocksci::ArbitraryLengthData<MultisigData> data(multisigData);
    for (auto &pubkeyScript : output.addresses) {
        data.add(pubkeyScript.scriptNum);
    }
    file.write(data);
    
    for (auto &pubkeyScript : output.addresses) {
        serialize(pubkeyScript);
    }
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::MULTISIG> &, ScriptFile<ScriptType::Enum::MULTISIG> &) {
    return ProcessedInput{};
}

void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::NONSTANDARD> &output, ScriptFile<ScriptType::Enum::NONSTANDARD> &file) {
    NonstandardScriptData scriptData(output.script);
    blocksci::ArbitraryLengthData<NonstandardScriptData> data(scriptData);
    data.add(output.script.begin(), output.script.end());
    file.write(data);
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::NONSTANDARD> &input, ScriptFile<ScriptType::Enum::NONSTANDARD> &file) {
    NonstandardScriptData scriptData(input.data.script);
    blocksci::ArbitraryLengthData<NonstandardScriptData> data(scriptData);
    data.add(input.data.script.begin(), input.data.script.end());
    file.write<1>(input.scriptNum - 1, data);
    return ProcessedInput{Script(input.scriptNum, ScriptType::Enum::NONSTANDARD)};
}

void AddressWriter::serializeImp(const ScriptData<AddressType::Enum::NULL_DATA> &output, ScriptFile<ScriptType::Enum::NULL_DATA> &file) {
    RawData scriptData(output.fullData);
    blocksci::ArbitraryLengthData<RawData> data(scriptData);
    data.add(output.fullData.begin(), output.fullData.end());
    file.write(data);
}

ProcessedInput AddressWriter::serializeImp(const ScriptInput<AddressType::Enum::NULL_DATA> &input, ScriptFile<ScriptType::Enum::NULL_DATA> &) {
    return ProcessedInput{Script(input.scriptNum, ScriptType::Enum::NULL_DATA)};
}

void AddressWriter::rollback(const blocksci::State &state) {
    blocksci::for_each(blocksci::ScriptInfoList(), [&](auto tag) {
        auto &file = std::get<ScriptFile<tag()>>(scriptFiles);
        file.truncate(state.scriptCounts[static_cast<size_t>(tag)]);
    });
}
