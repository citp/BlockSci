//
//  script_output.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#ifndef script_output_hpp
#define script_output_hpp

#include "basic_types.hpp"
#include "address_state.hpp"
#include "parser_fwd.hpp"

#include <blocksci/scripts/bitcoin_pubkey.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/scripts/script_view.hpp>

#include <mpark/variant.hpp>

#include <array>

template<blocksci::AddressType::Enum type>
struct ScriptOutput {
    static constexpr auto address_v = type;
    static constexpr auto script_v = scriptType(type);
    ScriptOutputData<type> data;
    uint32_t scriptNum;
    bool isNew;
    
    ScriptOutput() = default;
    ScriptOutput(const ScriptOutputData<type> &data_) : data(data_) {}
    
    template<blocksci::AddressType::Enum t = type, std::enable_if_t<blocksci::ScriptInfo<scriptType(t)>::deduped, int> = 0>
    uint32_t resolve(AddressState &state) {
        RawScript rawScript{data.getHash(), script_v};
        auto addressInfo = state.findAddress(rawScript);
        std::tie(scriptNum, isNew) = state.resolveAddress(addressInfo);
        assert(scriptNum > 0);
        if (isNew) {
            data.visitWrapped([&](auto &output) { output.resolve(state); });
        }
        return scriptNum;
    }
    
    template<blocksci::AddressType::Enum t = type, std::enable_if_t<!blocksci::ScriptInfo<scriptType(t)>::deduped, int> = 0>
    uint32_t resolve(AddressState &state) {
        scriptNum = state.getNewAddressIndex(script_v);
        isNew = true;
        assert(scriptNum > 0);
        data.visitWrapped([&](auto &output) { output.resolve(state); });
        return scriptNum;
    }
    
    template<blocksci::AddressType::Enum t = type, std::enable_if_t<blocksci::ScriptInfo<scriptType(t)>::deduped, int> = 0>
    void check(const AddressState &state) {
        RawScript rawScript{data.getHash(), script_v};
        auto addressInfo = state.findAddress(rawScript);
        scriptNum = addressInfo.addressNum;
        isNew = addressInfo.addressNum == 0;
        data.visitWrapped([&](auto &output) { output.check(state); });
    }
    
    template<blocksci::AddressType::Enum t = type, std::enable_if_t<!blocksci::ScriptInfo<scriptType(t)>::deduped, int> = 0>
    void check(const AddressState &state) {
        scriptNum = 0;
        isNew = true;
        data.visitWrapped([&](auto &output) { output.check(state); });
    }
};

struct ScriptOutputDataBase {
    bool isValid() const { return true; }
    
    template<typename Func>
    void visitWrapped(Func) {}
    
    template<typename Func>
    void visitWrapped(Func) const {}
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::PUBKEY> : public ScriptOutputDataBase {
    blocksci::CPubKey pubkey;
    
    ScriptOutputData(const boost::iterator_range<const unsigned char *> &vch1);
    ScriptOutputData(const blocksci::CPubKey &pub) : pubkey(pub) {}
    ScriptOutputData() = default;
    
    blocksci::uint160 getHash() const;
    
    blocksci::PubkeyData getData(uint32_t txNum) const;
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::PUBKEYHASH> : public ScriptOutputDataBase {
    
    blocksci::CKeyID hash;
    
    ScriptOutputData(blocksci::uint160 &pubkeyHash) : hash{pubkeyHash} {}
    
    blocksci::uint160 getHash() const;
    
    blocksci::PubkeyData getData(uint32_t txNum) const;
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH> : public ScriptOutputDataBase {
    
    blocksci::CKeyID hash;
    
    ScriptOutputData(blocksci::uint160 &&pubkeyHash) : hash{pubkeyHash} {}
    
    blocksci::uint160 getHash() const;
    
    blocksci::PubkeyData getData(uint32_t txNum) const;
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::SCRIPTHASH> : public ScriptOutputDataBase {
    blocksci::CKeyID hash;
    
    ScriptOutputData(blocksci::uint160 hash_) : hash(hash_) {}
    
    blocksci::uint160 getHash() const;
    
    blocksci::ScriptHashData getData(uint32_t txNum) const;
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH> : public ScriptOutputDataBase {
    blocksci::uint256 hash;
    
    ScriptOutputData(blocksci::uint256 hash_) : hash(hash_) {}
    
    blocksci::uint160 getHash() const;
    
    blocksci::ScriptHashData getData(uint32_t txNum) const;
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::MULTISIG> : public ScriptOutputDataBase {
    static constexpr int MAX_ADDRESSES = 16;
    uint8_t numRequired;
    uint8_t numTotal;
    uint16_t addressCount;
    
    std::vector<ScriptOutput<blocksci::AddressType::Enum::PUBKEY>> addresses;
    
    ScriptOutputData() : addressCount(0) {}
    
    void addAddress(const boost::iterator_range<const unsigned char *> &vch1);
    
    bool isValid() const {
        return numRequired <= numTotal && numTotal == addressCount;
    }
    
    blocksci::uint160 getHash() const;
    
    template<typename Func>
    void visitWrapped(Func func) {
        for (auto &pubkey : addresses) {
            func(pubkey);
        }
    }
    
    template<typename Func>
    void visitWrapped(Func func) const {
        for (auto &pubkey : addresses) {
            func(pubkey);
        }
    }
    
    blocksci::ArbitraryLengthData<blocksci::MultisigData> getData(uint32_t txNum) const;
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::NONSTANDARD> : public ScriptOutputDataBase {
    blocksci::CScriptView script;
    
    ScriptOutputData() {}
    ScriptOutputData(const blocksci::CScriptView &script);
    
    blocksci::ArbitraryLengthData<blocksci::NonstandardScriptData> getData(uint32_t txNum) const;
};

template <>
struct ScriptOutputData<blocksci::AddressType::Enum::NULL_DATA> : public ScriptOutputDataBase {
    std::vector<unsigned char> fullData;
    
    ScriptOutputData(const blocksci::CScriptView &script);
    
    blocksci::ArbitraryLengthData<blocksci::RawData> getData(uint32_t txNum) const;
};

using ScriptOutputType = blocksci::to_variadic_t<blocksci::to_address_tuple_t<ScriptOutput>, mpark::variant>;

class AnyScriptOutput {
public:
    ScriptOutputType wrapped;
    blocksci::Address address() const;
    bool isNew() const;
    blocksci::AddressType::Enum type() const;
    
    AnyScriptOutput() = default;
    AnyScriptOutput(const blocksci::CScriptView &scriptPubKey, bool witnessActivated);
    
    void check(const AddressState &state);
    uint32_t resolve(AddressState &state);
    bool isValid() const;
};

#endif /* script_output_hpp */
