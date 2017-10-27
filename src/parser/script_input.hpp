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
#include "script_processor.hpp"

#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/variant/variant.hpp>

#include <bitset>

class AddressWriter;
struct RawTransaction;
struct WitnessStackItem;
class CPubKey;

using ProcessedInput = boost::container::small_vector<uint32_t, 3>;

struct InputView {
    uint32_t inputNum;
    uint32_t txNum;
    const std::vector<WitnessStackItem> &witnessStack;
    bool witnessActivated;
    
    InputView(uint32_t inputNum_, uint32_t txNum_, const std::vector<WitnessStackItem> &witnessStack_, bool witnessActivated_) : inputNum(inputNum_), txNum(txNum_), witnessStack(witnessStack_), witnessActivated(witnessActivated_) {}
};

struct InputScriptView {
    uint32_t scriptNum;
    const unsigned char *scriptBegin;
    uint32_t scriptLength;
    
    InputScriptView(uint32_t scriptNum_, const unsigned char *scriptBegin_, uint32_t scriptLength_) : scriptNum(scriptNum_), scriptBegin(scriptBegin_), scriptLength(scriptLength_) {}
    
    CScriptView getScript() const {
        return CScriptView(scriptBegin, scriptBegin + scriptLength);
    }
};

struct ScriptInputBase {
    void checkInput(const InputView &, const InputScriptView &, const RawTransaction &, const AddressState &, const AddressWriter &) {}
};

template<auto>
struct ScriptInput;

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEY> : public ScriptInputBase {
    ScriptInput(const InputView &, const InputScriptView &, const RawTransaction &, const AddressWriter &) {}
    ProcessedInput processInput(const InputView &, const InputScriptView &, const RawTransaction &, AddressState &, AddressWriter &) {
        return ProcessedInput{};
    }
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;
    
    ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressWriter &);
    ProcessedInput processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;
    ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressWriter &);
    ProcessedInput processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptInputBase {
    CScript script;
    
    ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressWriter &);
    ProcessedInput processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

template <>
struct ScriptInput<blocksci::AddressType::Enum::NULL_DATA> : public ScriptInputBase {
    ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressWriter &);
    ProcessedInput processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

using ScriptInputType = blocksci::to_address_variant_t<ScriptInput>;

template<>
struct ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptInputBase {
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    const unsigned char *wrappedInputBegin;
    uint32_t wrappedInputLength;
    
    ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressWriter &);
    ProcessedInput processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &writer);
    void checkInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptInputBase {
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    
    ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressWriter &);
    ProcessedInput processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &writer);
    void checkInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::MULTISIG> : public ScriptInputBase {
    static constexpr int MAX_ADDRESSES = 16;
    std::bitset<MAX_ADDRESSES> spendSet;
    
    ScriptInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressWriter &);
    ProcessedInput processInput(const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

ScriptInputType generateScriptInput(const blocksci::Address &address, const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressWriter &addressWriter);

ProcessedInput processScriptInput(ScriptInputType &input, const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter);

void checkInput(blocksci::AddressType::Enum type, const InputView &inputView, const InputScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &addressWriter);

#endif /* script_input_hpp */
