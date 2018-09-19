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

bool isValidPubkey(ranges::iterator_range<const unsigned char *> &vch1) {
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

ScriptOutputDataType extractScriptData(const blocksci::CScriptView &scriptPubKey, bool witnessActivated) {
    using blocksci::AddressType;
    using blocksci::CScript;
    using blocksci::CScriptView;
    using blocksci::uint160;
    using blocksci::uint256;
    
    // Templates
    static std::vector<std::pair<AddressType::Enum, CScript>> &mTemplates = *[]() {
        auto templates = new std::vector<std::pair<AddressType::Enum, CScript>>{};
        // Standard tx, sender provides pubkey, receiver adds signature
        auto pubkey = std::make_pair(AddressType::Enum::PUBKEY, CScript() << blocksci::OP_PUBKEY << blocksci::OP_CHECKSIG);
        templates->push_back(pubkey);
        
        // Bitcoin address tx, sender provides hash of pubkey, receiver provides signature and pubkey
        auto pubkeyHash = std::make_pair(AddressType::Enum::PUBKEYHASH, CScript() << blocksci::OP_DUP << blocksci::OP_HASH160 << blocksci::OP_PUBKEYHASH << blocksci::OP_EQUALVERIFY << blocksci::OP_CHECKSIG);
        templates->push_back(pubkeyHash);
        
        // Sender provides N pubkeys, receivers provides M signatures
        auto multisig = std::make_pair(AddressType::Enum::MULTISIG, CScript() << blocksci::OP_SMALLINTEGER << blocksci::OP_PUBKEYS << blocksci::OP_SMALLINTEGER << blocksci::OP_CHECKMULTISIG);
        templates->push_back(multisig);
        return templates;
    }();
    
    // Shortcut for pay-to-script-hash, which are more constrained than the other types:
    // it is always OP_HASH160 20 [20 byte hash] OP_EQUAL
    if (scriptPubKey.IsPayToScriptHash()) {
        blocksci::uint160 hash;
        memcpy(&hash, &(*(scriptPubKey.begin()+2)), 20);
        return ScriptOutputData<AddressType::Enum::SCRIPTHASH>{{hash}};
    }
    
    if (witnessActivated && scriptPubKey.IsWitnessProgram()) {
        auto pc = scriptPubKey.begin();
        blocksci::opcodetype opcode;
        ranges::iterator_range<const unsigned char *> vchSig;
        scriptPubKey.GetOp(pc, opcode, vchSig);
        auto version = static_cast<uint8_t>(CScript::DecodeOP_N(opcode));
        scriptPubKey.GetOp(pc, opcode, vchSig);
        if (version == 0 && vchSig.size() == 20) {
            return ScriptOutputData<AddressType::Enum::WITNESS_PUBKEYHASH>(uint160{vchSig.begin(), vchSig.end()});
        } else if (version == 0 && vchSig.size() == 32) {
            return ScriptOutputData<AddressType::Enum::WITNESS_SCRIPTHASH>(uint256{vchSig.begin(), vchSig.end()});
        }
    }
    
    // Provably prunable, data-carrying output
    //
    // So long as script passes the IsUnspendable() test and all but the first
    // byte passes the IsPushOnly() test we don't care what exactly is in the
    // script.
    
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == blocksci::OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
        return ScriptOutputData<AddressType::Enum::NULL_DATA>{scriptPubKey};
    }
    
    // Scan templates
    const CScriptView& script1 = scriptPubKey;
    
    
    ranges::optional<ScriptOutputDataType> type;
    for (auto &tplate : mTemplates) {
        uint8_t numRequired = 0;
        bool isFirstSmallInt = true;
        
        const CScript& script2 = tplate.second;
        
        blocksci::opcodetype opcode1, opcode2;
        ranges::iterator_range<const unsigned char *> vch1;
        std::vector<unsigned char> vch2;
        
        // Compare
        auto pc1 = script1.begin();
        auto pc2 = script2.begin();
        while (true)
        {
            if (pc1 == script1.end() && pc2 == script2.end()) {
                if (!type || !mpark::visit([&](auto &data) { return data.isValid(); }, *type)) {
                    break;
                }
                return *type;
            }
            if (!script1.GetOp(pc1, opcode1, vch1)) {
                break;
            }
            if (!script2.GetOp(pc2, opcode2, vch2)) {
                break;
            }
            
            // Template matching opcodes:
            if (opcode2 == blocksci::OP_PUBKEYS) {
                ScriptOutputData<AddressType::Enum::MULTISIG> output;
                output.numRequired = numRequired;
                
                while (vch1.size() >= 33 && vch1.size() <= 65) {
                    output.addAddress(vch1);
                    
                    if (!script1.GetOp(pc1, opcode1, vch1)) {
                        break;
                    }
                }
                if (!script2.GetOp(pc2, opcode2, vch2)) {
                    break;
                }
                
                type = output;
            }
            
            if (opcode2 == blocksci::OP_PUBKEY) {
                if (!isValidPubkey(vch1)) {
                    break;
                }
                type = ScriptOutputData<AddressType::Enum::PUBKEY>{vch1};
            } else if (opcode2 == blocksci::OP_PUBKEYHASH) {
                if (vch1.size() != sizeof(uint160)) {
                    break;
                }
                auto address = uint160{vch1.begin(), vch1.end()};
                type = ScriptOutputData<AddressType::Enum::PUBKEYHASH>{address};
            } else if (opcode2 == blocksci::OP_SMALLINTEGER) {   // Single-byte small integer pushed onto vSolutions
                if (opcode1 == blocksci::OP_0 || (opcode1 >= blocksci::OP_1 && opcode1 <= blocksci::OP_16)) {
                    if (isFirstSmallInt) {
                        numRequired = static_cast<uint8_t>(CScript::DecodeOP_N(opcode1));
                        isFirstSmallInt = false;
                    } else {
                        auto &out = mpark::get<ScriptOutputData<AddressType::Enum::MULTISIG>>(*type);
                        out.numTotal = static_cast<uint8_t>(CScript::DecodeOP_N(opcode1));
                    }
                } else {
                    break;
                }
            } else if (opcode1 != opcode2 || !std::equal(vch1.begin(), vch1.end(), vch2.begin())) {
                // Others must match exactly
                break;
            }
        }
    }
    return ScriptOutputData<AddressType::Enum::NONSTANDARD>{scriptPubKey};
}

struct ScriptOutputGenerator {
    template <blocksci::AddressType::Enum type>
    ScriptOutputType operator()(const ScriptOutputData<type> &outputData) const {
        return ScriptOutput<type>(outputData);
    }
};
                     
AnyScriptOutput::AnyScriptOutput(const blocksci::CScriptView &scriptPubKey, bool witnessActivated) : wrapped(mpark::visit(ScriptOutputGenerator(), extractScriptData(scriptPubKey, witnessActivated))) {}

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

void AnyScriptOutput::check(AddressState &state) {
    mpark::visit([&](auto &output) { return output.check(state); }, wrapped);
}

uint32_t AnyScriptOutput::resolve(AddressState &state) {
    return mpark::visit([&](auto &output) { return output.resolve(state); }, wrapped);
}

// MARK: TX_PUBKEY

ScriptOutputData<blocksci::AddressType::Enum::PUBKEY>::ScriptOutputData(const ranges::iterator_range<const unsigned char *> &vch1) {
    std::copy(vch1.begin(), vch1.end(), pubkey.begin());
}

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::PUBKEY>::getHash() const {
    return hash160(pubkey.data(), pubkey.size());
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::PUBKEY>::getData(uint32_t txNum) const {
    return {txNum, pubkey};
}

// MARK: TX_PUBKEYHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::PUBKEYHASH>::getHash() const {
    return hash;
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::PUBKEYHASH>::getData(uint32_t txNum) const {
    return {txNum, hash};
}

// MARK: MULTISIG_PUBKEY

ScriptOutputData<blocksci::AddressType::Enum::MULTISIG_PUBKEY>::ScriptOutputData(const ranges::iterator_range<const unsigned char *> &vch1) {
    std::copy(vch1.begin(), vch1.end(), pubkey.begin());
}

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::MULTISIG_PUBKEY>::getHash() const {
    return hash160(pubkey.data(), pubkey.size());
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::MULTISIG_PUBKEY>::getData(uint32_t txNum) const {
    blocksci::RawPubkey rawPubkey;
    std::copy(pubkey.begin(), pubkey.end(), rawPubkey.begin());
    return {txNum, rawPubkey};
}

// MARK: WITNESS_PUBKEYHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getHash() const {
    return hash;
}

blocksci::PubkeyData ScriptOutputData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getData(uint32_t txNum) const {
    return {txNum, hash};
}

// MARK: TX_SCRIPTHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::SCRIPTHASH>::getHash() const {
    return hash;
}

blocksci::ScriptHashData ScriptOutputData<blocksci::AddressType::Enum::SCRIPTHASH>::getData(uint32_t txNum) const {
    return {txNum, hash, blocksci::RawAddress{0, blocksci::AddressType::Enum::NONSTANDARD}};
}

// MARK: WITNESS_SCRIPTHASH

blocksci::uint160 ScriptOutputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getHash() const {
    return ripemd160(reinterpret_cast<const char *>(&hash), sizeof(hash));
}

blocksci::ScriptHashData ScriptOutputData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getData(uint32_t txNum) const {
    return {txNum, hash, blocksci::RawAddress{0, blocksci::AddressType::Enum::NONSTANDARD}};
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

void ScriptOutputData<blocksci::AddressType::Enum::MULTISIG>::addAddress(const ranges::iterator_range<const unsigned char *> &vch1) {
    blocksci::RawPubkey pubkey;
    std::copy(vch1.begin(), vch1.end(), pubkey.begin());
    addresses.emplace_back(pubkey);
    addressCount++;
}

blocksci::ArbitraryLengthData<blocksci::MultisigData> ScriptOutputData<blocksci::AddressType::Enum::MULTISIG>::getData(uint32_t txNum) const {
    blocksci::MultisigData multisigData{txNum, numRequired, numTotal, addressCount};
    blocksci::ArbitraryLengthData<blocksci::MultisigData> data(multisigData);
    for (auto &pubkeyScript : addresses) {
        data.add(pubkeyScript.scriptNum);
    }
    return data;
}

// MARK: TX_NONSTANDARD

ScriptOutputData<blocksci::AddressType::Enum::NONSTANDARD>::ScriptOutputData(const blocksci::CScriptView &script_) : script(script_) {}

blocksci::ArbitraryLengthData<blocksci::NonstandardScriptData> ScriptOutputData<blocksci::AddressType::Enum::NONSTANDARD>::getData(uint32_t txNum) const {
    blocksci::NonstandardScriptData scriptData(txNum, static_cast<uint32_t>(script.size()));
    blocksci::ArbitraryLengthData<blocksci::NonstandardScriptData> data(scriptData);
    data.add(script.begin(), script.end());
    return data;
}

// MARK: TX_NULL_DATA

ScriptOutputData<blocksci::AddressType::Enum::NULL_DATA>::ScriptOutputData(const blocksci::CScriptView &script){
    blocksci::CScriptView::const_iterator pc1 = script.begin();
    blocksci::opcodetype opcode1;
    ranges::iterator_range<const unsigned char *> vch1;
    while(true) {
        if(!script.GetOp(pc1, opcode1, vch1)) {
            break;
        }
        fullData.insert(fullData.end(), vch1.begin(), vch1.end());
    }
}

blocksci::ArbitraryLengthData<blocksci::RawData> ScriptOutputData<blocksci::AddressType::Enum::NULL_DATA>::getData(uint32_t txNum) const {
    blocksci::RawData scriptData(txNum, fullData);
    blocksci::ArbitraryLengthData<blocksci::RawData> data(scriptData);
    data.add(fullData.begin(), fullData.end());
    return data;
}
