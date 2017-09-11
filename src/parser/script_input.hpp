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

#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <boost/variant/variant.hpp>

#include <bitset>
#include <stdio.h>

class AddressWriter;
struct RawTransaction;
struct WitnessStackItem;
class CPubKey;


struct InputInfo {
    uint32_t inputNum;
    const unsigned char *scriptBegin;
    uint32_t scriptLength;
    const std::vector<WitnessStackItem> &witnessStack;
    
    InputInfo(uint32_t inputNum_, const unsigned char *scriptBegin_, uint32_t scriptLength_, const std::vector<WitnessStackItem> &witnessStack_) : inputNum(inputNum_), scriptBegin(scriptBegin_), scriptLength(scriptLength_), witnessStack(witnessStack_) {}
    
    CScript getScript() const {
        return CScript(scriptBegin, scriptBegin + scriptLength);
    }
};

struct ScriptInputBase {
    void checkInput(const InputInfo &, const RawTransaction &, const AddressState &, const AddressWriter &) {}
};

template<blocksci::AddressType::Enum type>
struct ScriptInput;

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEY> : public ScriptInputBase {
    ScriptInput(const InputInfo &, const RawTransaction &, const AddressWriter &) {}
    void processInput(uint32_t, const InputInfo &, const RawTransaction &, AddressState &, AddressWriter &) {}
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(uint32_t addressNum, const InputInfo &, const RawTransaction &, AddressState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(uint32_t addressNum, const InputInfo &, const RawTransaction &, AddressState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptInputBase {
    CScript script;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(uint32_t addressNum, const InputInfo &, const RawTransaction &, AddressState &, AddressWriter &);
};

template <>
struct ScriptInput<blocksci::AddressType::Enum::NULL_DATA> : public ScriptInputBase {
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(uint32_t addressNum, const InputInfo &, const RawTransaction &, AddressState &, AddressWriter &);
};

using ScriptInputType = blocksci::to_script_variant_t<ScriptInput, blocksci::AddressInfoList>;

template<>
struct ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptInputBase {
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    const unsigned char *wrappedInputBegin;
    uint32_t wrappedInputLength;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(uint32_t addressNum, const InputInfo &inputInfo, const RawTransaction &tx, AddressState &state, AddressWriter &writer);
    void checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptInputBase {
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(uint32_t addressNum, const InputInfo &inputInfo, const RawTransaction &tx, AddressState &state, AddressWriter &writer);
    void checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::MULTISIG> : public ScriptInputBase {
    static constexpr int MAX_ADDRESSES = 16;
    std::bitset<MAX_ADDRESSES> spendSet;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(uint32_t addressNum, const InputInfo &, const RawTransaction &, AddressState &, AddressWriter &);
};

#endif /* script_input_hpp */
