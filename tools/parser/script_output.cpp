//
//  script_output.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_output.hpp"

#include <internal/hash.hpp>

#include <blocksci/scripts/bitcoin_pubkey.hpp>

using namespace blocksci;

bool isValidPubkey(ranges::subrange<const unsigned char *> &vch1) {
    if (vch1.size() < 33 || vch1.size() > 65) {
        return false;
    }
    
    auto chHeader = static_cast<char>(vch1[0]);
    if ((chHeader == 2 || chHeader == 3) && vch1.size() == 33) {
        return true;
    } else if ((chHeader == 4 || chHeader == 6 || chHeader == 7) && vch1.size() == 65) {
        return true;
    }
    
    return false;
}

using ScriptOutputDataType = blocksci::to_variadic_t<blocksci::to_address_tuple_t<ScriptOutputData>, mpark::variant>;
using valtype = ranges::subrange<const unsigned char *>;

static bool MatchPayToPubkey(const CScriptView& script, valtype& pubkey)
{
    if (script.size() == CPubKey::PUBLIC_KEY_SIZE + 2 && script[0] == CPubKey::PUBLIC_KEY_SIZE && script.back() == OP_CHECKSIG) {
        pubkey = valtype(script.begin() + 1, script.begin() + blocksci::CPubKey::PUBLIC_KEY_SIZE + 1);
        return CPubKey::ValidSize(pubkey | ranges::to_vector);
    }
    if (script.size() == CPubKey::COMPRESSED_PUBLIC_KEY_SIZE + 2 && script[0] == CPubKey::COMPRESSED_PUBLIC_KEY_SIZE && script.back() == OP_CHECKSIG) {
        pubkey = valtype(script.begin() + 1, script.begin() + CPubKey::COMPRESSED_PUBLIC_KEY_SIZE + 1);
        return CPubKey::ValidSize(pubkey | ranges::to_vector);
    }
    return false;
}

static bool MatchPayToPubkeyHash(const CScriptView& script, valtype& pubkeyhash)
{
    if (script.size() == 25 && script[0] == OP_DUP && script[1] == OP_HASH160 && script[2] == 20 && script[23] == OP_EQUALVERIFY && script[24] == OP_CHECKSIG) {
        pubkeyhash = valtype(script.begin () + 3, script.begin() + 23);
        return true;
    }
    return false;
}

/** Test for "small positive integer" script opcodes - OP_1 through OP_16. */
static constexpr bool IsSmallInteger(opcodetype opcode)
{
    return opcode >= OP_1 && opcode <= OP_16;
}

static bool MatchMultisig(const CScriptView& script, ScriptOutputData<blocksci::AddressType::Enum::MULTISIG> &multisig)
{
    opcodetype opcode;
    valtype data;
    CScriptView::iterator it = script.begin();
    if (script.size() < 1 || script.back() != OP_CHECKMULTISIG) return false;
    
    if (!script.GetOp(it, opcode, data) || !IsSmallInteger(opcode)) return false;
    multisig.numRequired = CScript::DecodeOP_N(opcode);
    while (script.GetOp(it, opcode, data) && CPubKey::ValidSize(data | ranges::to_vector)) {
        multisig.addAddress(data);
    }
    if (!IsSmallInteger(opcode)) return false;
    multisig.numTotal = CScript::DecodeOP_N(opcode);
    return (it + 1 == script.end());
}


ScriptOutputDataType extractScriptData(const blocksci::CScriptView &scriptPubKey, bool p2shActivated, bool witnessActivated) {
    using blocksci::AddressType;
    using blocksci::CScript;
    using blocksci::CScriptView;
    using blocksci::uint160;
    using blocksci::uint256;
    
    // Shortcut for pay-to-script-hash, which are more constrained than the other types:
    // it is always OP_HASH160 20 [20 byte hash] OP_EQUAL
    if (p2shActivated && scriptPubKey.IsPayToScriptHash()) {
        blocksci::uint160 hash;
        memcpy(&hash, &(*(scriptPubKey.begin()+2)), 20);
        return ScriptOutputData<AddressType::Enum::SCRIPTHASH>{{hash}};
    }
    
    uint8_t witnessversion;
    ranges::subrange<const unsigned char *> witnessprogram;
    if (witnessActivated && scriptPubKey.IsWitnessProgram(witnessversion, witnessprogram)) {
        if (witnessversion == 0) {
            if (witnessprogram.size() == 20) {
                return ScriptOutputData<AddressType::Enum::WITNESS_PUBKEYHASH>(uint160{witnessprogram.begin(), witnessprogram.end()});
            } else if (witnessprogram.size() == 32) {
                return ScriptOutputData<AddressType::Enum::WITNESS_SCRIPTHASH>(uint256{witnessprogram.begin(), witnessprogram.end()});
            } else {
                // Witness v0 with other script length is treated as nonstandard
                return ScriptOutputData<AddressType::Enum::NONSTANDARD>{scriptPubKey};
            }
        } else {
             return ScriptOutputData<AddressType::Enum::WITNESS_UNKNOWN>(witnessversion, witnessprogram);
        }
    }
    
    // Provably prunable, data-carrying output
    //
    // So long as script passes the IsUnspendable() test and all but the first
    // byte passes the IsPushOnly() test we don't care what exactly is in the
    // script.
    
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
        return ScriptOutputData<AddressType::Enum::NULL_DATA>{scriptPubKey};
    }
    
    ranges::subrange<const unsigned char *> data;
    if (MatchPayToPubkey(scriptPubKey, data)) {
        return ScriptOutputData<AddressType::Enum::PUBKEY>{data};
    }
    
    if (MatchPayToPubkeyHash(scriptPubKey, data)) {
        auto address = uint160{data.begin(), data.end()};
        return ScriptOutputData<AddressType::Enum::PUBKEYHASH>{address};
    }
    
    ScriptOutputData<blocksci::AddressType::Enum::MULTISIG> multisigOutput;
    if (MatchMultisig(scriptPubKey, multisigOutput)) {
        return multisigOutput;
    }
    
    return ScriptOutputData<AddressType::Enum::NONSTANDARD>{scriptPubKey};
}

struct ScriptOutputGenerator {
    template <blocksci::AddressType::Enum type>
    ScriptOutputType operator()(const ScriptOutputData<type> &outputData) const {
        return ScriptOutput<type>(outputData);
    }
};
                     
AnyScriptOutput::AnyScriptOutput(const blocksci::CScriptView &scriptPubKey, bool p2shActivated, bool witnessActivated) : wrapped(mpark::visit(ScriptOutputGenerator(), extractScriptData(scriptPubKey, p2shActivated, witnessActivated))) {}

blocksci::RawAddress AnyScriptOutput::address() const {
    return mpark::visit([&](auto &output) { return blocksci::RawAddress{output.scriptNum, output.address_v}; }, wrapped);
}

bool AnyScriptOutput::isNew() const {
    return mpark::visit([&](auto &output) { return output.isNew; }, wrapped);
}

bool AnyScriptOutput::isValid() const {
    return mpark::visit([&](auto &output) { return output.data.isValid(); }, wrapped);
}

blocksci::AddressType::Enum AnyScriptOutput::type() const {
    return mpark::visit([&](auto &output) { return output.address_v; }, wrapped);
}

uint32_t AnyScriptOutput::resolve(AddressState &state) {
    return mpark::visit([&](auto &output) { return output.resolve(state); }, wrapped);
}

// MARK: TX_PUBKEY

ScriptOutputData<blocksci::AddressType::Enum::PUBKEY>::ScriptOutputData(const ranges::subrange<const unsigned char *> &vch1) {
    pubkey.fill(0);
    std::copy(vch1.begin(), vch1.end(), pubkey.begin());
}

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::PUBKEY>::getHash() const {
    auto length = CPubKey::GetLen(pubkey[0]);
    return hash160(pubkey.data(), length);
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::PUBKEY>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::PubkeyData data{txNum, pubkey};
    data.saw(blocksci::AddressType::Enum::PUBKEY, topLevel);
    return data;
}

// MARK: TX_PUBKEYHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::PUBKEYHASH>::getHash() const {
    return hash;
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::PUBKEYHASH>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::PubkeyData data{txNum, hash};
    data.saw(blocksci::AddressType::Enum::PUBKEYHASH, topLevel);
    return data;
}

// MARK: MULTISIG_PUBKEY

ScriptOutputData<blocksci::AddressType::Enum::MULTISIG_PUBKEY>::ScriptOutputData(const ranges::subrange<const unsigned char *> &vch1) {
    std::copy(vch1.begin(), vch1.end(), pubkey.begin());
}

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::MULTISIG_PUBKEY>::getHash() const {
    auto length = CPubKey::GetLen(pubkey[0]);
    return hash160(pubkey.data(), length);
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::MULTISIG_PUBKEY>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::PubkeyData data{txNum, pubkey};
    data.saw(blocksci::AddressType::Enum::MULTISIG_PUBKEY, topLevel);
    return data;
}

// MARK: WITNESS_PUBKEYHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getHash() const {
    return hash;
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::PubkeyData data{txNum, hash};
    data.saw(blocksci::AddressType::Enum::WITNESS_PUBKEYHASH, topLevel);
    return data;
}

// MARK: TX_SCRIPTHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::SCRIPTHASH>::getHash() const {
    return hash;
}

blocksci::ScriptHashData ScriptOutputData<blocksci::AddressType::Enum::SCRIPTHASH>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::ScriptHashData data{txNum, hash, blocksci::RawAddress{0, blocksci::AddressType::Enum::NONSTANDARD}};
    data.saw(blocksci::AddressType::Enum::SCRIPTHASH, topLevel);
    return data;
}

// MARK: WITNESS_SCRIPTHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getHash() const {
    return ripemd160(reinterpret_cast<const char *>(&hash), sizeof(hash));
}

blocksci::ScriptHashData ScriptOutputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::ScriptHashData data{txNum, hash, blocksci::RawAddress{0, blocksci::AddressType::Enum::NONSTANDARD}};
    data.saw(blocksci::AddressType::Enum::WITNESS_SCRIPTHASH, topLevel);
    return data;
}

// MARK: TX_MULTISIG

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::MULTISIG>::getHash() const {
    std::vector<char> sigData;
    sigData.resize(sizeof(numRequired) + sizeof(blocksci::uint160) * addressCount);
    size_t sigDataPos = 0;
    
    memcpy(&sigData[sigDataPos], reinterpret_cast<const char *>(&numRequired), sizeof(numRequired));
    sigDataPos += sizeof(numRequired);
    
    std::vector<blocksci::RawPubkey> pubkeys;
    pubkeys.reserve(addresses.size());
    for (auto &output : addresses) {
        pubkeys.push_back(output.data.pubkey);
    }
    
    std::sort(pubkeys.begin(), pubkeys.end());
    
    for (auto &pubkey : pubkeys) {
        auto addressHash = hash160(pubkey.data(), pubkey.size());
        memcpy(&sigData[sigDataPos], reinterpret_cast<const char *>(&addressHash), sizeof(addressHash));
        sigDataPos += sizeof(addressHash);
    }
    
    return ripemd160(sigData.data(), sigData.size());
}

void ScriptOutputData<blocksci::AddressType::Enum::MULTISIG>::addAddress(const ranges::subrange<const unsigned char *> &vch1) {
    blocksci::RawPubkey pubkey;
    pubkey.fill(0);
    std::copy(vch1.begin(), vch1.end(), pubkey.begin());
    addresses.emplace_back(pubkey);
    addressCount++;
}

blocksci::ArbitraryLengthData<blocksci::MultisigData> ScriptOutputData<blocksci::AddressType::Enum::MULTISIG>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::MultisigData multisigData{txNum, numRequired, numTotal, addressCount};
    multisigData.saw(blocksci::AddressType::Enum::MULTISIG, topLevel);
    blocksci::ArbitraryLengthData<blocksci::MultisigData> data(multisigData);
    for (auto &pubkeyScript : addresses) {
        data.add(pubkeyScript.scriptNum);
    }
    return data;
}

// MARK: TX_NONSTANDARD

ScriptOutputData<blocksci::AddressType::Enum::NONSTANDARD>::ScriptOutputData(const blocksci::CScriptView &script_) : script(script_) {}

blocksci::ArbitraryLengthData<blocksci::NonstandardScriptData> ScriptOutputData<blocksci::AddressType::Enum::NONSTANDARD>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::NonstandardScriptData scriptData(txNum, static_cast<uint32_t>(script.size()));
    scriptData.saw(blocksci::AddressType::Enum::NONSTANDARD, topLevel);
    blocksci::ArbitraryLengthData<blocksci::NonstandardScriptData> data(scriptData);
    data.add(script.begin(), script.end());
    return data;
}

// MARK: TX_NULL_DATA

ScriptOutputData<blocksci::AddressType::Enum::NULL_DATA>::ScriptOutputData(const blocksci::CScriptView &script){
    blocksci::CScriptView::iterator pc1 = script.begin();
    blocksci::opcodetype opcode1;
    ranges::subrange<const unsigned char *> vch1;
    while(true) {
        if(!script.GetOp(pc1, opcode1, vch1)) {
            break;
        }
        fullData.insert(fullData.end(), vch1.begin(), vch1.end());
    }
}

blocksci::ArbitraryLengthData<blocksci::RawData> ScriptOutputData<blocksci::AddressType::Enum::NULL_DATA>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::RawData scriptData(txNum, fullData);
    scriptData.saw(blocksci::AddressType::Enum::NULL_DATA, topLevel);
    blocksci::ArbitraryLengthData<blocksci::RawData> data(scriptData);
    data.add(fullData.begin(), fullData.end());
    return data;
}

// MARK: WITNESS_UNKNOWN

ScriptOutputData<blocksci::AddressType::Enum::WITNESS_UNKNOWN>::ScriptOutputData(uint8_t witnessVersion_, const ranges::subrange<const unsigned char *> &witnessData_) : witnessVersion(witnessVersion_) {
    witnessData.assign(witnessData_.begin(), witnessData_.end());
}

blocksci::ArbitraryLengthData<blocksci::WitnessUnknownScriptData> ScriptOutputData<blocksci::AddressType::Enum::WITNESS_UNKNOWN>::getData(uint32_t txNum, bool topLevel) const {
    blocksci::WitnessUnknownScriptData scriptData(txNum, witnessVersion, static_cast<uint32_t>(witnessData.size()));
    scriptData.saw(blocksci::AddressType::Enum::WITNESS_UNKNOWN, topLevel);
    blocksci::ArbitraryLengthData<blocksci::WitnessUnknownScriptData> data(scriptData);
    data.add(witnessData.begin(), witnessData.end());
    return data;
}
