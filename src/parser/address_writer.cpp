//
//  address_writer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_writer.hpp"

AddressWriter::AddressWriter(const ParserConfigurationBase &config) :
scriptFiles(blocksci::apply(blocksci::ScriptInfoList(), [&] (auto tag) {
    return config.scriptsDirectory()/ std::string{scriptName(tag)};
})) {
}

struct SerializeOutputVisitor : public boost::static_visitor<void> {
    AddressWriter &writer;
    
    SerializeOutputVisitor(AddressWriter &writer_) : writer(writer_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(const ScriptOutput<type> &data) const
    {
        writer.serialize(data);
    }
};

struct SerializeInputVisitor : public boost::static_visitor<void> {
    AddressWriter &writer;
    uint32_t scriptNum;
    
    SerializeInputVisitor(AddressWriter &writer_, uint32_t scriptNum_) : writer(writer_), scriptNum(scriptNum_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(const ScriptInput<type> &data) const {
        writer.serialize(data, scriptNum);
    }
};

void AddressWriter::serialize(const ScriptOutputType &output) {
    SerializeOutputVisitor visitor(*this);
    boost::apply_visitor(visitor, output);
}

void AddressWriter::serialize(const ScriptInputType &input, uint32_t scriptNum) {
    SerializeInputVisitor visitor(*this, scriptNum);
    boost::apply_visitor(visitor, input);
}

using namespace blocksci;

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEY>(const ScriptOutput<AddressType::Enum::PUBKEY> &output) {
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({output.pubkey, output.pubkey.GetID()});
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::PUBKEY>(const ScriptInput<AddressType::Enum::PUBKEY> &, uint32_t) {
    return false;
}

constexpr static CPubKey nullPubkey{};

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEYHASH>(const ScriptOutput<AddressType::Enum::PUBKEYHASH> &output) {
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({nullPubkey, output.hash});
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::PUBKEYHASH>(const ScriptInput<AddressType::Enum::PUBKEYHASH> &input, uint32_t scriptNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    auto data = file.getData(scriptNum - 1);
    if (data->pubkey == nullPubkey) {
        data->pubkey = input.pubkey;
        return true;
    }
    return false;
}


template<>
void AddressWriter::serializeImp<AddressType::Enum::WITNESS_PUBKEYHASH>(const ScriptOutput<AddressType::Enum::WITNESS_PUBKEYHASH> &output) {
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({nullPubkey, output.hash});
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::WITNESS_PUBKEYHASH>(const ScriptInput<AddressType::Enum::WITNESS_PUBKEYHASH> &input, uint32_t scriptNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    auto data = file.getData(scriptNum - 1);
    if (data->pubkey == nullPubkey) {
        data->pubkey = input.pubkey;
        return true;
    }
    return false;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::WITNESS_SCRIPTHASH>(const ScriptOutput<AddressType::Enum::WITNESS_SCRIPTHASH> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    blocksci::Address wrappedAddress;
    file.write({output.getHash(), wrappedAddress, 0});
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::WITNESS_SCRIPTHASH>(const ScriptInput<AddressType::Enum::WITNESS_SCRIPTHASH> &input, uint32_t scriptNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    auto data = file.getData(scriptNum - 1);
    if (data->wrappedAddress.scriptNum == 0) {
        data->wrappedAddress = input.wrappedAddress;
        data->txRevealed = input.txNum;
        return true;
    }
    if (input.containsNewOutput) {
        serialize(input.wrappedScriptOutput);
    }
    return false;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::SCRIPTHASH>(const ScriptOutput<AddressType::Enum::SCRIPTHASH> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    blocksci::Address wrappedAddress;
    file.write({output.hash, wrappedAddress, 0});
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::SCRIPTHASH>(const ScriptInput<AddressType::Enum::SCRIPTHASH> &input, uint32_t scriptNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    auto data = file.getData(scriptNum - 1);
    if (data->wrappedAddress.scriptNum == 0) {
        data->wrappedAddress = input.wrappedAddress;
        data->txRevealed = input.txNum;
        return true;
    }
    if (input.containsNewOutput) {
        serialize(input.wrappedScriptOutput);
    }
    return false;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::MULTISIG>(const ScriptOutput<AddressType::Enum::MULTISIG> &output) {
    blocksci::Address wrappedAddress;
    auto &multisigFile = std::get<ScriptFile<ScriptType::Enum::MULTISIG>>(scriptFiles);
    multisigFile.writeIndexGroup();
    MultisigData data{output.numRequired, output.numTotal, output.addressCount};
    bool clearedData = multisigFile.write(data);
    for (uint8_t i = 0; i < output.addressCount; i++) {
        clearedData |= multisigFile.write(output.processedAddresses[i]);
    }
    if (clearedData) {
        multisigFile.clearBuffer();
    }
    
    for (size_t i = 0; i < output.addressCount; i++) {
        ScriptOutput<blocksci::AddressType::Enum::PUBKEY> pubkeyOutput{output.addresses[i]};
        if (output.firstSeen[i]) {
            serialize(pubkeyOutput);
        }
    }
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::MULTISIG>(const ScriptInput<AddressType::Enum::MULTISIG> &, uint32_t) {
    return false;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::NONSTANDARD>(const ScriptOutput<AddressType::Enum::NONSTANDARD> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.writeIndexGroup();
    file.write(output.script.begin(), output.script.end());
    
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::NONSTANDARD>(const ScriptInput<AddressType::Enum::NONSTANDARD> &input, uint32_t scriptNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.updateIndexGroup<1>(scriptNum - 1);
    file.write(input.script.begin(), input.script.end());
    return true;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::NULL_DATA>(const ScriptOutput<AddressType::Enum::NULL_DATA> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NULL_DATA>>(scriptFiles);
    file.writeIndexGroup();
    file.write(output.fullData.begin(), output.fullData.end());
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::NULL_DATA>(const ScriptInput<AddressType::Enum::NULL_DATA> &, uint32_t) {
    return true;
}

void AddressWriter::rollback(const blocksci::State &state) {
    blocksci::for_each(blocksci::ScriptInfoList(), [&](auto tag) {
        auto &file = std::get<ScriptFile<tag()>>(scriptFiles);
        file.truncate(state.scriptCounts[static_cast<size_t>(tag)]);
    });
}
