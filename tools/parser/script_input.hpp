//
//  script_input.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef script_input_hpp
#define script_input_hpp

#include "parser_fwd.hpp"
#include "script_output.hpp"

#include <internal/script_view.hpp>

#include <mpark/variant.hpp>

#include <bitset>

struct InputView {
    uint32_t inputNum;
    uint32_t txNum;
    std::vector<WitnessStackItem> witnessStack;
    bool witnessActivated;
    
    InputView(uint32_t inputNum_, uint32_t txNum_, std::vector<WitnessStackItem> && witnessStack_, bool witnessActivated_) : inputNum(inputNum_), txNum(txNum_), witnessStack(std::move(witnessStack_)), witnessActivated(witnessActivated_) {}
};

template<blocksci::AddressType::Enum type>
struct ScriptInput {
    static constexpr auto address_v = type;
    
    ScriptInputData<type> data;
    uint32_t scriptNum;
    
    
    ScriptInput(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<type> &spendData) : data(inputView, scriptView, tx, spendData) {}
    
    void process(AddressState &state) {
        data.process(state);
    }
};

struct ScriptInputDataBase {
    void process(AddressState &) {}
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::PUBKEY> : public ScriptInputDataBase {
    ScriptInputData(const InputView &, const blocksci::CScriptView &, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::PUBKEY> &) {}
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptInputDataBase {
    blocksci::RawPubkey pubkey;

    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::PUBKEYHASH> &);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::MULTISIG_PUBKEY> : public ScriptInputDataBase {
    ScriptInputData(const InputView &, const blocksci::CScriptView &, const RawTransaction &, const SpendData<blocksci::AddressType::Enum::MULTISIG_PUBKEY> &) {}
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptInputDataBase {
    blocksci::RawPubkey pubkey;
    
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> &);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptInputDataBase {
    blocksci::CScript script;
    
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

template<>
struct ScriptInputData<blocksci::AddressType::Enum::WITNESS_UNKNOWN> : public ScriptInputDataBase {
    blocksci::CScript script;
    
    ScriptInputData() = default;
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_UNKNOWN> &);
};

class AnyScriptInput;

template<>
struct ScriptInputData<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptInputDataBase {
    AnyScriptOutput wrappedScriptOutput;
    std::unique_ptr<AnyScriptInput> wrappedScriptInput;
    
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::SCRIPTHASH> &);
    
    void process(AddressState &state);

private:
    ScriptInputData(std::pair<AnyScriptOutput, std::unique_ptr<AnyScriptInput>> data);
};

template<>
struct ScriptInputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptInputDataBase {
    AnyScriptOutput wrappedScriptOutput;
    std::unique_ptr<AnyScriptInput> wrappedScriptInput;
    
    ScriptInputData(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const SpendData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> &);
    
    void process(AddressState &state);

private:
    ScriptInputData(std::pair<AnyScriptOutput, std::unique_ptr<AnyScriptInput>> data);
};

using ScriptInputType = blocksci::to_variadic_t<blocksci::to_address_tuple_t<ScriptInput>, mpark::variant>;

class AnyScriptInput {
public:
    ScriptInputType wrapped;
    
    AnyScriptInput() = default;
    AnyScriptInput(const InputView &inputView, const blocksci::CScriptView &scriptView, const RawTransaction &tx, const AnySpendData &spendData);
    
    void process(AddressState &state);

    void setScriptNum(uint32_t scriptNum);
    
    blocksci::RawAddress address() const;
};

#endif /* script_input_hpp */
