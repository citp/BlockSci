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
#include "parser_fwd.hpp"

#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/scripts/script_view.hpp>
#include <blocksci/chain/output_pointer.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <bitset>

struct InputView {
    uint32_t inputNum;
    uint32_t txNum;
    const std::vector<WitnessStackItem> &witnessStack;
    bool witnessActivated;
    
    InputView(uint32_t inputNum_, uint32_t txNum_, const std::vector<WitnessStackItem> &witnessStack_, bool witnessActivated_) : inputNum(inputNum_), txNum(txNum_), witnessStack(witnessStack_), witnessActivated(witnessActivated_) {}
};

template<auto type>
struct ScriptInput {
    ScriptInputData<type> data;
    uint32_t scriptNum;
    
    ScriptInput(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<type> &spendData) : data(inputView, scriptView, tx, spendData) {}
    
    void process(AddressState &state) {
        data.process(state);
    }
    
    void check(const AddressState &state) {
        data.check(state);
    }
};

struct ScriptInputDataBase {
    uint32_t txNum;
    
    ScriptInputDataBase() = default;
    ScriptInputDataBase(const InputView &inputView, const blocksci::CScriptView &) : txNum(inputView.txNum) {}
    void check(const AddressState &) {}
    void process(AddressState &) {}
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::PUBKEY> : public ScriptInputDataBase {
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::PUBKEY> &) : ScriptInputDataBase(inputView, scriptView) {}
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptInputDataBase {
    CPubKey pubkey;

    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::PUBKEYHASH> &);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptInputDataBase {
    CPubKey pubkey;
    
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> &);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptInputDataBase {
    CScript script;
    
    ScriptInputData() = default;
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::NONSTANDARD> &);
};

template <>
struct ScriptInputData<blocksci::AddressType::Enum::NULL_DATA> : public ScriptInputDataBase {
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::NULL_DATA> &);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::MULTISIG> : public ScriptInputDataBase {
    static constexpr int MAX_ADDRESSES = 16;
    std::bitset<MAX_ADDRESSES> spendSet;
    
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::MULTISIG> &spendData);
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
    AnyScriptInput(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const AnySpendData &spendData);
    
    void process(AddressState &state);
    void check(const AddressState &state);
    
    void setScriptNum(uint32_t scriptNum);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptInputDataBase {
    AnyScriptOutput wrappedScriptOutput;
    AnyScriptInput wrappedScriptInput;
    
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::SCRIPTHASH> &);
    
    void process(AddressState &state);
    void check(const AddressState &state);
    
private:
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const std::pair<AnyScriptOutput, AnyScriptInput> &data);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptInputDataBase {
    AnyScriptOutput wrappedScriptOutput;
    AnyScriptInput wrappedScriptInput;
    
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &);
    
    void process(AddressState &state);
    void check(const AddressState &state);
    
private:
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const std::pair<AnyScriptOutput, AnyScriptInput> &data);
};

#endif /* script_input_hpp */
