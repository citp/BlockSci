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
    const InputInfo &inputInfo;
    
    SerializeInputVisitor(AddressWriter &writer_, const InputInfo &inputInfo_) : writer(writer_), inputInfo(inputInfo_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(const ScriptInput<type> &data) const {
        writer.serialize(data, inputInfo);
    }
};

void AddressWriter::serialize(const ScriptOutputType &output) {
    SerializeOutputVisitor visitor(*this);
    boost::apply_visitor(visitor, output);
}

void AddressWriter::serialize(const ScriptInputType &input, const InputInfo &inputInfo) {
    SerializeInputVisitor visitor(*this, inputInfo);
    boost::apply_visitor(visitor, input);
}

using namespace blocksci;

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEY>(const ScriptOutput<AddressType::Enum::PUBKEY> &output) {
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({output.pubkey, output.pubkey.GetID()});
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::PUBKEY>(const ScriptInput<AddressType::Enum::PUBKEY> &, const InputInfo &) {
    return false;
}

constexpr static CPubKey nullPubkey{};

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEYHASH>(const ScriptOutput<AddressType::Enum::PUBKEYHASH> &output) {
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({nullPubkey, output.hash});
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::PUBKEYHASH>(const ScriptInput<AddressType::Enum::PUBKEYHASH> &input, const InputInfo &inputInfo) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    auto data = file.getData(inputInfo.addressNum - 1);
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
bool AddressWriter::serializeImp<AddressType::Enum::WITNESS_PUBKEYHASH>(const ScriptInput<AddressType::Enum::WITNESS_PUBKEYHASH> &input, const InputInfo &inputInfo) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    auto data = file.getData(inputInfo.addressNum - 1);
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
bool AddressWriter::serializeImp<AddressType::Enum::WITNESS_SCRIPTHASH>(const ScriptInput<AddressType::Enum::WITNESS_SCRIPTHASH> &input, const InputInfo &inputInfo) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    auto data = file.getData(inputInfo.addressNum - 1);
    if (data->wrappedAddress.addressNum == 0) {
        data->wrappedAddress = input.wrappedAddress;
        data->txRevealed = inputInfo.txNum;
        return true;
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
bool AddressWriter::serializeImp<AddressType::Enum::SCRIPTHASH>(const ScriptInput<AddressType::Enum::SCRIPTHASH> &input, const InputInfo &inputInfo) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    auto data = file.getData(inputInfo.addressNum - 1);
    if (data->wrappedAddress.addressNum == 0) {
        data->wrappedAddress = input.wrappedAddress;
        data->txRevealed = inputInfo.txNum;
        return true;
    }
    return false;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::MULTISIG>(const ScriptOutput<AddressType::Enum::MULTISIG> &output) {
    blocksci::Address wrappedAddress;
    auto &file = std::get<ScriptFile<ScriptType::Enum::MULTISIG>>(scriptFiles);
    file.writeIndexGroup();
    MultisigData data{output.numRequired, output.numTotal, output.addressCount};
    bool clearedData = file.write(data);
    for (uint8_t i = 0; i < output.addressCount; i++) {
        clearedData |= file.write(output.processedAddresses[i]);
    }
    if (clearedData) {
        file.clearBuffer();
    }
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::MULTISIG>(const ScriptInput<AddressType::Enum::MULTISIG> &, const InputInfo &) {
    return false;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::NONSTANDARD>(const ScriptOutput<AddressType::Enum::NONSTANDARD> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.writeIndexGroup();
    file.write(output.script.begin(), output.script.end());
    
}

template<>
bool AddressWriter::serializeImp<AddressType::Enum::NONSTANDARD>(const ScriptInput<AddressType::Enum::NONSTANDARD> &input, const InputInfo &inputInfo) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.updateIndexGroup<1>(inputInfo.addressNum - 1);
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
bool AddressWriter::serializeImp<AddressType::Enum::NULL_DATA>(const ScriptInput<AddressType::Enum::NULL_DATA> &, const InputInfo &) {
    return true;
}

void AddressWriter::rollback(const blocksci::State &state) {
    blocksci::for_each(blocksci::ScriptInfoList(), [&](auto tag) {
        auto &file = std::get<ScriptFile<tag()>>(scriptFiles);
        file.truncate(state.scriptCounts[static_cast<size_t>(tag)]);
    });
}
