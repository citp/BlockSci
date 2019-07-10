//
//  address_writer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef address_writer_hpp
#define address_writer_hpp

#include "script_output.hpp"
#include "script_input.hpp"

template<typename T>
struct ScriptFileType;

template<typename T>
struct ScriptFileType<blocksci::FixedSize<T>> {
    using type = blocksci::FixedSizeFileMapper<T, mio::access_mode::write>;
};

template<typename ...T>
struct ScriptFileType<blocksci::Indexed<T...>> {
    using type = blocksci::IndexedFileMapper<mio::access_mode::write, T...>;
};

template<blocksci::DedupAddressType::Enum type>
using ScriptFileType_t = typename ScriptFileType<typename blocksci::ScriptInfo<type>::storage>::type;

template<blocksci::DedupAddressType::Enum type>
struct ScriptFile : public ScriptFileType_t<type> {
    using ScriptFileType_t<type>::ScriptFileType_t;
};


class AddressWriter {
    using ScriptFilesTuple = blocksci::to_dedup_address_tuple_t<ScriptFile>;
    
    ScriptFilesTuple scriptFiles;
    
    template<blocksci::AddressType::Enum type>
    void serializeImp(const ScriptInput<type> &, ScriptFile<dedupType(type)> &) {}
    
    void serializeImp(const ScriptInput<blocksci::AddressType::PUBKEYHASH> &input, ScriptFile<blocksci::DedupAddressType::PUBKEY> &file);
    void serializeImp(const ScriptInput<blocksci::AddressType::WITNESS_PUBKEYHASH> &input, ScriptFile<blocksci::DedupAddressType::PUBKEY> &file);
    void serializeImp(const ScriptInput<blocksci::AddressType::SCRIPTHASH> &input, ScriptFile<blocksci::DedupAddressType::SCRIPTHASH> &file);
    void serializeImp(const ScriptInput<blocksci::AddressType::WITNESS_SCRIPTHASH> &input, ScriptFile<blocksci::DedupAddressType::SCRIPTHASH> &file);
    void serializeImp(const ScriptInput<blocksci::AddressType::NONSTANDARD> &input, ScriptFile<blocksci::DedupAddressType::NONSTANDARD> &file);
    void serializeImp(const ScriptInput<blocksci::AddressType::WITNESS_UNKNOWN> &input, ScriptFile<blocksci::DedupAddressType::WITNESS_UNKNOWN> &file);
    
    template<blocksci::AddressType::Enum type>
    void serializeImp(const ScriptOutput<type> &output, ScriptFile<dedupType(type)> &file, bool topLevel) {
        auto data = file[output.scriptNum - 1];
        data->saw(type, topLevel);
    }
    
    void serializeImp(const ScriptOutput<blocksci::AddressType::PUBKEY> &output, ScriptFile<blocksci::DedupAddressType::PUBKEY> &file, bool topLevel);
    void serializeImp(const ScriptOutput<blocksci::AddressType::MULTISIG_PUBKEY> &output, ScriptFile<blocksci::DedupAddressType::PUBKEY> &file, bool topLevel);
    void serializeImp(const ScriptOutput<blocksci::AddressType::WITNESS_SCRIPTHASH> &output, ScriptFile<blocksci::DedupAddressType::SCRIPTHASH> &file, bool topLevel);
    void serializeImp(const ScriptOutput<blocksci::AddressType::NONSTANDARD> &output, ScriptFile<blocksci::DedupAddressType::NONSTANDARD> &file, bool topLevel);
    void serializeImp(const ScriptOutput<blocksci::AddressType::WITNESS_UNKNOWN> &output, ScriptFile<blocksci::DedupAddressType::WITNESS_UNKNOWN> &file, bool topLevel);
    
    template<blocksci::AddressType::Enum type>
    void serializeWrapped(const ScriptInputData<type> &, uint32_t, uint32_t) {}
    
    void serializeWrapped(const ScriptInputData<blocksci::AddressType::Enum::SCRIPTHASH> &input, uint32_t txNum, uint32_t outputTxNum);
    void serializeWrapped(const ScriptInputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &input, uint32_t txNum, uint32_t outputTxNum);
    
public:
    
    template <blocksci::DedupAddressType::Enum type>
    ScriptFile<type> &getFile() {
        return std::get<ScriptFile<type>>(scriptFiles);
    }
    
    template <blocksci::DedupAddressType::Enum type>
    const ScriptFile<type> &getFile() const {
        return std::get<ScriptFile<type>>(scriptFiles);
    }
    
    template<blocksci::AddressType::Enum type>
    blocksci::OffsetType serializeNew(const ScriptOutput<type> &output, uint32_t txNum, bool topLevel) {
        assert(output.isNew);
        auto &file = std::get<ScriptFile<dedupType(type)>>(scriptFiles);
        auto data = output.data.getData(txNum, topLevel);
        file.write(data);
        assert(output.scriptNum == file.size());
        output.data.visitWrapped([&](auto &wrappedOutput) {
            if (wrappedOutput.isNew) {
                serializeNew(wrappedOutput, txNum, false);
            } else {
                serializeExisting(wrappedOutput, false);
            }
        });
        return file.size();
    }
    
    template<blocksci::AddressType::Enum type>
    void serializeExisting(const ScriptOutput<type> &output, bool topLevel) {
        assert(!output.isNew);
        auto &file = std::get<ScriptFile<dedupType(type)>>(scriptFiles);
        serializeImp(output, file, topLevel);
    }
    
    template<blocksci::AddressType::Enum type>
    void serialize(const ScriptInput<type> &input, uint32_t txNum, uint32_t outputTxNum) {
        auto &file = std::get<ScriptFile<dedupType(type)>>(scriptFiles);
        auto data = file.getDataAtIndex(input.scriptNum - 1);

        /** Default value of ScriptDataBase.txFirstSpent is (std::numeric_limits<uint32_t>::max()) */
        bool isFirstSpend = data->txFirstSpent == std::numeric_limits<uint32_t>::max();

        /** Default value of ScriptDataBase.txFirstSeen is the txNum of the transaction that contained the script */
        bool isNewerFirstSeen = outputTxNum < data->txFirstSeen;

        if (isNewerFirstSeen) {
            data->txFirstSeen = outputTxNum;
        }
        if (isFirstSpend) {
            data->txFirstSpent = txNum;
            serializeImp(input, file);
        }
    }
    
    void rollback(const blocksci::State &state);
    
    blocksci::OffsetType serializeNew(const AnyScriptOutput &output, uint32_t txNum, bool topLevel);
    void serializeExisting(const AnyScriptOutput &output, bool topLevel);
    
    void serialize(const AnyScriptInput &input, uint32_t txNum, uint32_t outputTxNum);
    void serializeWrapped(const AnyScriptInput &input, uint32_t txNum, uint32_t outputTxNum);
    
    AddressWriter(const ParserConfigurationBase &config);
};

#endif /* address_writer_hpp */
