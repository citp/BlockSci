//
//  address_writer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//
#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_writer.hpp"

AddressWriter::AddressWriter(const ParserConfiguration &config) :
scriptFiles(blocksci::apply(blocksci::ScriptInfoList(), [&] (auto tag) {
    return config.scriptsDirectory()/blocksci::ScriptInfo<decltype(tag)::type>::typeName;
})) {
}

struct SerializeOutputVisitor : public boost::static_visitor<void> {
    AddressWriter &writer;
    
    SerializeOutputVisitor(AddressWriter &writer_) : writer(writer_) {}
    template <blocksci::ScriptType::Enum type>
    void operator()(const ScriptOutput<type> &data) const
    {
        writer.serialize(data);
    }
};

struct SerializeInputVisitor : public boost::static_visitor<void> {
    AddressWriter &writer;
    
    SerializeInputVisitor(AddressWriter &writer_) : writer(writer_) {}
    template <blocksci::ScriptType::Enum type>
    void operator()(const ScriptInput<type> &data) const {
        writer.serialize(data);
    }
};

void AddressWriter::serialize(const ScriptOutputType &output) {
    SerializeOutputVisitor visitor(*this);
    boost::apply_visitor(visitor, output);
}

void AddressWriter::serialize(const ScriptInputType &input) {
    SerializeInputVisitor visitor(*this);
    boost::apply_visitor(visitor, input);
}

using namespace blocksci;

template<>
void AddressWriter::serializeImp<ScriptType::Enum::PUBKEY>(const ScriptOutput<ScriptType::Enum::PUBKEY> &output) {
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({output.pubkey});
    auto &pubkeyHashFile = std::get<ScriptFile<ScriptType::Enum::PUBKEYHASH>>(scriptFiles);
    pubkeyHashFile.write({output.pubkey.GetID()});
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::PUBKEY>(const ScriptInput<ScriptType::Enum::PUBKEY> &) {

}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::PUBKEYHASH>(const ScriptOutput<ScriptType::Enum::PUBKEYHASH> &output) {
    static std::vector<unsigned char> nullPubkeyVec(64, 0xff);
    static CPubKey nullPubkey{nullPubkeyVec};
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({nullPubkey});
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEYHASH>>(scriptFiles);
    file.write({output.hash});
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::PUBKEYHASH>(const ScriptInput<ScriptType::Enum::PUBKEYHASH> &input) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    auto data = file.getData(input.addressNum - 1);
    data->pubkey = input.pubkey;
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::SCRIPTHASH>(const ScriptOutput<ScriptType::Enum::SCRIPTHASH> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    blocksci::AddressPointer wrappedAddress;
    file.write({output.hash, wrappedAddress});
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::SCRIPTHASH>(const ScriptInput<ScriptType::Enum::SCRIPTHASH> &input) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    auto data = file.getData(input.addressNum - 1);
    data->wrappedAddress = input.wrappedAddress;
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::MULTISIG>(const ScriptOutput<ScriptType::Enum::MULTISIG> &output) {
    blocksci::AddressPointer wrappedAddress;
    auto &file = std::get<ScriptFile<ScriptType::Enum::MULTISIG>>(scriptFiles);
    file.writeIndexGroup();
    MultisigData data{output.numRequired, output.numTotal, output.addressCount};
    bool clearedData = file.write(data);
    for (uint8_t i = 0; i < output.addressCount; i++) {
        clearedData |= file.write(output.processedAddresses[i].addressNum);
    }
    if (clearedData) {
        file.clearBuffer();
    }
    
    for (uint8_t i = 0; i < output.addressCount; i++) {
        if (output.firstSeen[i]) {
            auto innerOutput = ScriptOutput<blocksci::ScriptType::Enum::PUBKEY>{output.addresses[i]};
            serialize(innerOutput);
        }
    }
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::MULTISIG>(const ScriptInput<ScriptType::Enum::MULTISIG> &) {
    
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::NONSTANDARD>(const ScriptOutput<ScriptType::Enum::NONSTANDARD> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.writeIndexGroup();
    file.write(output.script.begin(), output.script.end());
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::NONSTANDARD>(const ScriptInput<ScriptType::Enum::NONSTANDARD> &input) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.updateIndexGroup<1>(input.addressNum);
    file.write(input.script.begin(), input.script.end());
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::NULL_DATA>(const ScriptOutput<ScriptType::Enum::NULL_DATA> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NULL_DATA>>(scriptFiles);
    file.writeIndexGroup();
    file.write(output.fullData.begin(), output.fullData.end());
}

template<>
void AddressWriter::serializeImp<ScriptType::Enum::NULL_DATA>(const ScriptInput<ScriptType::Enum::NULL_DATA> &) {
    
}

template<blocksci::ScriptType::Enum type>
struct ScriptTruncateFunctor {
    static void f(uint32_t index, AddressWriter &writer) {
        writer.truncate<type>(index);
    }
};

void AddressWriter::truncate(blocksci::ScriptType::Enum type, uint32_t addressIndex) {
    static constexpr auto table = blocksci::make_dynamic_table<ScriptTruncateFunctor>();
    static constexpr std::size_t size = blocksci::ScriptType::all.size();
    
    auto index = static_cast<size_t>(type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](addressIndex, *this);
}

