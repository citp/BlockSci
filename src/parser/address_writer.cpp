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
    template <blocksci::AddressType::Enum type>
    void operator()(const ScriptOutput<type> &data) const
    {
        writer.serialize(data);
    }
};

struct SerializeInputVisitor : public boost::static_visitor<void> {
    AddressWriter &writer;
    uint32_t addressNum;
    
    SerializeInputVisitor(AddressWriter &writer_, uint32_t addressNum_) : writer(writer_), addressNum(addressNum_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(const ScriptInput<type> &data) const {
        writer.serialize(data, addressNum);
    }
};

void AddressWriter::serialize(const ScriptOutputType &output) {
    SerializeOutputVisitor visitor(*this);
    boost::apply_visitor(visitor, output);
}

void AddressWriter::serialize(const ScriptInputType &input, uint32_t addressNum) {
    SerializeInputVisitor visitor(*this, addressNum);
    boost::apply_visitor(visitor, input);
}

using namespace blocksci;

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEY>(const ScriptOutput<AddressType::Enum::PUBKEY> &output) {
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({output.pubkey, output.pubkey.GetID()});
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEY>(const ScriptInput<AddressType::Enum::PUBKEY> &, uint32_t) {

}

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEYHASH>(const ScriptOutput<AddressType::Enum::PUBKEYHASH> &output) {
    static std::vector<unsigned char> nullPubkeyVec(64, 0xff);
    static CPubKey nullPubkey{nullPubkeyVec};
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({nullPubkey, output.hash});
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::PUBKEYHASH>(const ScriptInput<AddressType::Enum::PUBKEYHASH> &input, uint32_t addressNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    auto data = file.getData(addressNum - 1);
    data->pubkey = input.pubkey;
}


template<>
void AddressWriter::serializeImp<AddressType::Enum::WITNESS_PUBKEYHASH>(const ScriptOutput<AddressType::Enum::WITNESS_PUBKEYHASH> &output) {
    static std::vector<unsigned char> nullPubkeyVec(64, 0xff);
    static CPubKey nullPubkey{nullPubkeyVec};
    auto &pubkeyFile = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    pubkeyFile.write({nullPubkey, output.hash});
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::WITNESS_PUBKEYHASH>(const ScriptInput<AddressType::Enum::WITNESS_PUBKEYHASH> &input, uint32_t addressNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::PUBKEY>>(scriptFiles);
    auto data = file.getData(addressNum - 1);
    data->pubkey = input.pubkey;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::WITNESS_SCRIPTHASH>(const ScriptOutput<AddressType::Enum::WITNESS_SCRIPTHASH> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    blocksci::Address wrappedAddress;
    file.write({output.getHash(), wrappedAddress});
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::WITNESS_SCRIPTHASH>(const ScriptInput<AddressType::Enum::WITNESS_SCRIPTHASH> &input, uint32_t addressNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    auto data = file.getData(addressNum - 1);
    data->wrappedAddress = input.wrappedAddress;
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::SCRIPTHASH>(const ScriptOutput<AddressType::Enum::SCRIPTHASH> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    blocksci::Address wrappedAddress;
    file.write({output.hash, wrappedAddress});
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::SCRIPTHASH>(const ScriptInput<AddressType::Enum::SCRIPTHASH> &input, uint32_t addressNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::SCRIPTHASH>>(scriptFiles);
    auto data = file.getData(addressNum - 1);
    data->wrappedAddress = input.wrappedAddress;
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
    
    for (uint8_t i = 0; i < output.addressCount; i++) {
        if (output.firstSeen[i]) {
            auto innerOutput = ScriptOutput<blocksci::AddressType::Enum::PUBKEY>{output.addresses[i]};
            serialize(innerOutput);
        }
    }
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::MULTISIG>(const ScriptInput<AddressType::Enum::MULTISIG> &, uint32_t) {
    
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::NONSTANDARD>(const ScriptOutput<AddressType::Enum::NONSTANDARD> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.writeIndexGroup();
    file.write(output.script.begin(), output.script.end());
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::NONSTANDARD>(const ScriptInput<AddressType::Enum::NONSTANDARD> &input, uint32_t addressNum) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NONSTANDARD>>(scriptFiles);
    file.updateIndexGroup<1>(addressNum);
    file.write(input.script.begin(), input.script.end());
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::NULL_DATA>(const ScriptOutput<AddressType::Enum::NULL_DATA> &output) {
    auto &file = std::get<ScriptFile<ScriptType::Enum::NULL_DATA>>(scriptFiles);
    file.writeIndexGroup();
    file.write(output.fullData.begin(), output.fullData.end());
}

template<>
void AddressWriter::serializeImp<AddressType::Enum::NULL_DATA>(const ScriptInput<AddressType::Enum::NULL_DATA> &, uint32_t) {
    
}

template<blocksci::ScriptType::Enum type>
struct ScriptTruncateFunctor {
    static void f(uint32_t index, AddressWriter &writer) {
        writer.truncate<type>(index);
    }
};

void AddressWriter::truncate(blocksci::ScriptType::Enum type, uint32_t addressIndex) {
    static constexpr auto table = blocksci::make_dynamic_table<blocksci::ScriptType, ScriptTruncateFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](addressIndex, *this);
}

