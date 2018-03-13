//
//  address_writer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef address_writer_hpp
#define address_writer_hpp

#include "parser_configuration.hpp"
#include "script_output.hpp"
#include "script_input.hpp"

#include <blocksci/util/file_mapper.hpp>
#include <blocksci/util/state.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script_data.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

template<typename T>
struct ScriptFileType;

template<typename T>
struct ScriptFileType<blocksci::FixedSize<T>> {
    using type = blocksci::FixedSizeFileMapper<T, blocksci::AccessMode::readwrite>;
};

template<typename ...T>
struct ScriptFileType<blocksci::Indexed<T...>> {
    using type = blocksci::IndexedFileMapper<blocksci::AccessMode::readwrite, T...>;
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
    
    template<blocksci::AddressType::Enum type>
    void serializeImp(const ScriptOutput<type> &, ScriptFile<dedupType(type)> &) {}
    
    void serializeImp(const ScriptOutput<blocksci::AddressType::PUBKEY> &input, ScriptFile<blocksci::DedupAddressType::PUBKEY> &file);
    void serializeImp(const ScriptOutput<blocksci::AddressType::WITNESS_SCRIPTHASH> &input, ScriptFile<blocksci::DedupAddressType::SCRIPTHASH> &file);
    
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
    size_t serialize(const ScriptOutput<type> &output, uint32_t txNum) {
        if (output.isNew) {
            auto &file = std::get<ScriptFile<dedupType(type)>>(scriptFiles);
            auto data = output.data.getData(txNum);
            file.write(data);
            output.data.visitWrapped([&](auto &output) { this->serialize(output, txNum); });
            return file.size();
        } else {
            auto &file = std::get<ScriptFile<dedupType(type)>>(scriptFiles);
            serializeImp(output, file);
        }
        return 0;
    }
    
    template<blocksci::AddressType::Enum type>
    void serialize(const ScriptInput<type> &input, uint32_t txNum, uint32_t outputTxNum) {
        auto &file = std::get<ScriptFile<dedupType(type)>>(scriptFiles);
        auto data = file.getDataAtIndex(input.scriptNum - 1);
        bool isFirstSpend = data->txFirstSpent == std::numeric_limits<uint32_t>::max();
        bool isNewerFirstSeen = outputTxNum < data->txFirstSeen;
        
        if (isNewerFirstSeen) {
            data->txFirstSeen = outputTxNum;
        }
        if (isFirstSpend) {
            data->txFirstSpent = txNum;
            serializeImp(input, file);
        }
        
        serializeWrapped(input.data, txNum, outputTxNum);
    }
    
    void rollback(const blocksci::State &state);
    
    size_t serialize(const AnyScriptOutput &output, uint32_t txNum);
    void serialize(const AnyScriptInput &input, uint32_t txNum, uint32_t outputTxNum);
    
    AddressWriter(const ParserConfigurationBase &config);
};

#endif /* address_writer_hpp */
