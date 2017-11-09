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
#include "parser_fwd.hpp"

#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <bitset>

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
    
    ScriptInputBase() = default;
    ScriptInputBase(const InputView &inputView, const CScriptView &) : txNum(inputView.txNum) {}
    void check(const AddressState &) {}
    void process(AddressState &) {}
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEY> : public ScriptInputBase {
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::PUBKEY> &) : ScriptInputBase(inputView, scriptView) {}
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;

    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::PUBKEYHASH> &);
    
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptInputBase {
    CPubKey pubkey;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> &);
    
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptInputBase {
    CScript script;
    
    ScriptInput() = default;
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::NONSTANDARD> &);
    
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &);
};

template <>
struct ScriptInput<blocksci::AddressType::Enum::NULL_DATA> : public ScriptInputBase {
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::NULL_DATA> &);

    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::MULTISIG> : public ScriptInputBase {
    static constexpr int MAX_ADDRESSES = 16;
    std::bitset<MAX_ADDRESSES> spendSet;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::MULTISIG> &spendData);
    
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &);
};

template<auto addressType>
struct VariantMemberType {
    using type = ScriptInput<addressType>;
};

template<>
struct VariantMemberType<blocksci::AddressType::Enum::SCRIPTHASH> {
    using type = boost::recursive_wrapper<ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH>>;
};

template<>
struct VariantMemberType<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> {
    using type = boost::recursive_wrapper<ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>>;
};

template<auto addressType>
using VariantMemberType_t = typename VariantMemberType<addressType>::type;

using ScriptInputType = blocksci::to_address_variant_t<VariantMemberType_t>;

class AnyScriptInput {
public:
    ScriptInputType wrapped;
    
    AnyScriptInput() = default;
    AnyScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const AnySpendData &spendData);
    
    void process(AddressState &state);
    void check(const AddressState &state);
    
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &writer);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptInputBase {
    AnyScriptOutput wrappedScriptOutput;
    AnyScriptInput wrappedScriptInput;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::SCRIPTHASH> &);
    
    void process(AddressState &state);
    void check(const AddressState &state);
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &writer);
    
private:
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const std::pair<AnyScriptOutput, AnyScriptInput> &data);
};

template<>
struct ScriptInput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptInputBase {
    AnyScriptOutput wrappedScriptOutput;
    AnyScriptInput wrappedScriptInput;
    
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &);
    
    void process(AddressState &state);
    void check(const AddressState &state);
    
    ProcessedInput serialize(uint32_t scriptNum, AddressWriter &writer);
    
private:
    ScriptInput(const InputView &inputView, const CScriptView &scriptView, const std::pair<AnyScriptOutput, AnyScriptInput> &data);
};

#endif /* script_input_hpp */
