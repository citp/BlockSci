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

struct ScriptInputBase {
    uint32_t txNum;
    
    ScriptInputBase(const InputView &inputView, const CScriptView &) : txNum(inputView.txNum) {}
    void checkInput(const InputView &, const CScriptView &, const RawTransaction &, const AddressState &, const AddressWriter &) {}
};

template<auto>
struct ScriptInput;

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEY> : public ScriptInputBase {
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &) : ScriptInputBase(inputView, scriptView) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &) : ScriptInput(inputView, scriptView, tx) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::PUBKEY> &) : ScriptInput(inputView, scriptView, tx) {}
    ProcessedInput processInput(uint32_t, const InputView &, const CScriptView &, const RawTransaction &, AddressState &, AddressWriter &) {
        return ProcessedInput{};
    }
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &);
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &) : ScriptInput(inputView, scriptView, tx) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::PUBKEYHASH> &) : ScriptInput(inputView, scriptView, tx) {}
    ProcessedInput processInput(uint32_t scriptNum, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &);
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &) : ScriptInput(inputView, scriptView, tx) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> &) : ScriptInput(inputView, scriptView, tx) {}
    
    ProcessedInput processInput(uint32_t scriptNum, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptInputBase {
    CScript script;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &);
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &) : ScriptInput(inputView, scriptView, tx) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::NONSTANDARD> &) : ScriptInput(inputView, scriptView, tx) {}
    
    ProcessedInput processInput(uint32_t scriptNum, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

template <>
struct ScriptInput<blocksci::AddressType::Enum::NULL_DATA> : public ScriptInputBase {
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &);
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &) : ScriptInput(inputView, scriptView, tx) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::NULL_DATA> &) : ScriptInput(inputView, scriptView, tx) {}

    ProcessedInput processInput(uint32_t scriptNum, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

using ScriptInputType = blocksci::to_address_variant_t<ScriptInput>;

template<>
struct ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptInputBase {
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    const unsigned char *wrappedInputBegin;
    uint32_t wrappedInputLength;
    bool containsNewOutput;
    std::unique_ptr<ScriptInputType> wrappedScriptInput;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &);
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &) : ScriptInput(inputView, scriptView, tx) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::SCRIPTHASH> &) : ScriptInput(inputView, scriptView, tx) {}

    ProcessedInput processInput(uint32_t scriptNum, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &writer);
    
    void checkInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptInputBase {
    blocksci::Address wrappedAddress;
    ScriptOutputType wrappedScriptOutput;
    bool containsNewOutput;
    std::unique_ptr<ScriptInputType> wrappedScriptInput;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &);
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &) : ScriptInput(inputView, scriptView, tx) {}
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &) : ScriptInput(inputView, scriptView, tx) {}
    
    ProcessedInput processInput(uint32_t scriptNum, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &writer);
    void checkInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::MULTISIG> : public ScriptInputBase {
    static constexpr int MAX_ADDRESSES = 16;
    std::bitset<MAX_ADDRESSES> spendSet;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressWriter &writer);
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const ScriptOutput<blocksci::AddressType::Enum::MULTISIG> &spentOutput);
    
    ProcessedInput processInput(uint32_t scriptNum, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, AddressState &, AddressWriter &);
};

ScriptInputType generateScriptInput(const blocksci::Address &address, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, AddressWriter &addressWriter);

ProcessedInput processScriptInput(uint32_t scriptNum, ScriptInputType &input, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, AddressState &state, AddressWriter &addressWriter);

void checkInput(blocksci::AddressType::Enum type, const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AddressState &state, const AddressWriter &addressWriter);

#endif /* script_input_hpp */
