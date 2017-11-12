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

#include <blocksci/file_mapper.hpp>
#include <blocksci/state.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script_data.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/scripts/scriptsfwd.hpp>

#include <boost/variant/variant.hpp>

using ProcessedInput = boost::container::small_vector<blocksci::Script, 3>;

template<typename T>
struct ScriptFileType;

template<typename T>
struct ScriptFileType<blocksci::FixedSize<T>> {
    using type = blocksci::FixedSizeFileMapper<T, boost::iostreams::mapped_file::readwrite>;
};

template<typename ...T>
struct ScriptFileType<blocksci::Indexed<T...>> {
    using type = blocksci::IndexedFileMapper<boost::iostreams::mapped_file::readwrite, T...>;
};


template<auto type>
using ScriptFileType_t = typename ScriptFileType<typename blocksci::ScriptInfo<type>::storage>::type;

template<auto type>
struct ScriptFile : public ScriptFileType_t<type> {
    using ScriptFileType_t<type>::ScriptFileType_t;
};


class AddressWriter {
    using ScriptFilesTuple = blocksci::to_script_tuple_t<ScriptFile>;
    
    ScriptFilesTuple scriptFiles;
    
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::PUBKEY> &input, ScriptFile<blocksci::ScriptType::Enum::PUBKEY> &file);
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH> &input, ScriptFile<blocksci::ScriptType::Enum::PUBKEY> &file);
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> &input, ScriptFile<blocksci::ScriptType::Enum::PUBKEY> &file);
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH> &input, ScriptFile<blocksci::ScriptType::Enum::SCRIPTHASH> &file);
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &input, ScriptFile<blocksci::ScriptType::Enum::SCRIPTHASH> &file);
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::MULTISIG> &input, ScriptFile<blocksci::ScriptType::Enum::MULTISIG> &file);
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::NULL_DATA> &input, ScriptFile<blocksci::ScriptType::Enum::NULL_DATA> &file);
    ProcessedInput serializeImp(const ScriptInput<blocksci::AddressType::Enum::NONSTANDARD> &input, ScriptFile<blocksci::ScriptType::Enum::NONSTANDARD> &file);
    
    template<auto type>
    void serializeImp(const ScriptData<type> &output, ScriptFile<scriptType(type)> &file) {
        file.write(output.getData());
    }
    
    void serializeImp(const ScriptData<blocksci::AddressType::Enum::MULTISIG> &input, ScriptFile<blocksci::ScriptType::Enum::MULTISIG> &file);
    
public:
    
    template <blocksci::ScriptType::Enum type>
    ScriptFile<type> &getFile() {
        return std::get<ScriptFile<type>>(scriptFiles);
    }
    
    template <blocksci::ScriptType::Enum type>
    const ScriptFile<type> &getFile() const {
        return std::get<ScriptFile<type>>(scriptFiles);
    }
    
    template<blocksci::AddressType::Enum type>
    uint32_t serialize(const ScriptOutput<type> &output) {
        if (output.isNew) {
            auto &file = std::get<ScriptFile<scriptType(type)>>(scriptFiles);
            serializeImp(output.data, file);
            return file.size();
        }
        return 0;
    }
    
    template<blocksci::AddressType::Enum type>
    ProcessedInput serialize(const ScriptInput<type> &input) {
        auto &file = std::get<ScriptFile<scriptType(type)>>(scriptFiles);
        return serializeImp(input, file);
    }
    
    void rollback(const blocksci::State &state);
    
    uint32_t serialize(const AnyScriptOutput &output);
    ProcessedInput serialize(const AnyScriptInput &input);
    
    AddressWriter(const ParserConfigurationBase &config);
};

#endif /* address_writer_hpp */
