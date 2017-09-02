//
//  script_input.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef script_input_hpp
#define script_input_hpp

#include "basic_types.hpp"
#include "script_output.hpp"

#include <blocksci/scripts/script_types.hpp>
#include <blocksci/scripts/address.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <boost/variant/variant.hpp>

#include <bitset>
#include <stdio.h>

class AddressWriter;
struct RawTransaction;
class CPubKey;


struct InputInfo {
    blocksci::AddressPointer address;
    uint32_t inputNum;
    const unsigned char *scriptBegin;
    const unsigned char *scriptEnd;
    
    InputInfo(const blocksci::AddressPointer &address_, uint32_t inputNum_, const unsigned char *scriptBegin_, const unsigned char *scriptEnd_) : address(address_), inputNum(inputNum_), scriptBegin(scriptBegin_), scriptEnd(scriptEnd_) {}
    
    CScript getScript() const {
        return CScript(scriptBegin, scriptEnd);
    }
};

struct ScriptInputBase {
    void checkInput(const InputInfo &, const RawTransaction &, const BlockchainState &, const AddressWriter &) {}
};

template<blocksci::ScriptType::Enum type>
struct ScriptInput;

template<>
struct ScriptInput<blocksci::ScriptType::Enum::PUBKEY> : public ScriptInputBase {
    ScriptInput(const InputInfo &, const RawTransaction &, const AddressWriter &) {}
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &) {}
};

template<>
struct ScriptInput<blocksci::ScriptType::Enum::PUBKEYHASH> : public ScriptInputBase {
    uint32_t addressNum;
    CPubKey pubkey;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::ScriptType::Enum::NONSTANDARD> : public ScriptInputBase {
    uint32_t addressNum;
    CScript script;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

template <>
struct ScriptInput<blocksci::ScriptType::Enum::NULL_DATA> : public ScriptInputBase {
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

using ScriptInputType = blocksci::to_script_variant_t<ScriptInput, blocksci::ScriptInfoList>;

template<>
struct ScriptInput<blocksci::ScriptType::Enum::SCRIPTHASH> : public ScriptInputBase {
    uint32_t addressNum;
    blocksci::AddressPointer wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    std::vector<unsigned char> wrappedInputScript;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &inputInfo, const RawTransaction &tx, BlockchainState &state, AddressWriter &writer);
    void checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const BlockchainState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::ScriptType::Enum::MULTISIG> : public ScriptInputBase {
    static constexpr int MAX_ADDRESSES = 16;
    uint32_t addressNum;
    std::bitset<MAX_ADDRESSES> spendSet;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

#endif /* script_input_hpp */
