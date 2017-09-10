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
    blocksci::Address address;
    uint32_t inputNum;
    const unsigned char *scriptBegin;
    const unsigned char *scriptEnd;
    const std::vector<WitnessStackItem> &witnessStack;
    
    InputInfo(const blocksci::Address &address_, uint32_t inputNum_, const unsigned char *scriptBegin_, const unsigned char *scriptEnd_, const std::vector<WitnessStackItem> &witnessStack_) : address(address_), inputNum(inputNum_), scriptBegin(scriptBegin_), scriptEnd(scriptEnd_), witnessStack(witnessStack_) {}
    
    CScript getScript() const {
        return CScript(scriptBegin, scriptEnd);
    }
};

struct ScriptInputBase {
    void checkInput(const InputInfo &, const RawTransaction &, const BlockchainState &, const AddressWriter &) {}
};

template<blocksci::AddressType::Enum type>
struct ScriptInput;

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEY> : public ScriptInputBase {
    ScriptInput(const InputInfo &, const RawTransaction &, const AddressWriter &) {}
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &) {}
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptInputBase {
    uint32_t addressNum;
    CPubKey pubkey;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptInputBase {
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptInputBase {
    uint32_t addressNum;
    CScript script;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

template <>
struct ScriptInput<blocksci::AddressType::Enum::NULL_DATA> : public ScriptInputBase {
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

using ScriptInputType = blocksci::to_script_variant_t<ScriptInput, blocksci::AddressInfoList>;

template<>
struct ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptInputBase {
    uint32_t addressNum;
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    std::vector<unsigned char> wrappedInputScript;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &inputInfo, const RawTransaction &tx, BlockchainState &state, AddressWriter &writer);
    void checkInput(const InputInfo &inputInfo, const RawTransaction &tx, const BlockchainState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptInputBase {
    uint32_t addressNum;
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    std::vector<unsigned char> wrappedInputScript;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &inputInfo, const RawTransaction &tx, BlockchainState &state, AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::MULTISIG> : public ScriptInputBase {
    static constexpr int MAX_ADDRESSES = 16;
    uint32_t addressNum;
    std::bitset<MAX_ADDRESSES> spendSet;
    
    ScriptInput(const InputInfo &inputInfo, const RawTransaction &tx, const AddressWriter &);
    void processInput(const InputInfo &, const RawTransaction &, BlockchainState &, AddressWriter &);
};

#endif /* script_input_hpp */
