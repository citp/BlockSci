//
//  script_output.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "script_output.hpp"
#include "address_writer.hpp"

#include <blocksci/hash.hpp>

#include <boost/variant/get.hpp>

bool isValidPubkey(boost::iterator_range<const unsigned char *> &vch1);

bool isValidPubkey(boost::iterator_range<const unsigned char *> &vch1) {
    if (vch1.size() < 33 || vch1.size() > 65)
        return false;
    
    char chHeader = static_cast<char>(vch1[0]);
    if ((chHeader == 2 || chHeader == 3) && vch1.size() == 33) {
        return true;
    } else if ((chHeader == 4 || chHeader == 6 || chHeader == 7) && vch1.size() == 65) {
        return true;
    }
    
    return false;
}

using ScriptDataType = blocksci::to_address_variant_t<ScriptData>;

ScriptDataType extractScriptData(const blocksci::CScriptView &scriptPubKey, bool witnessActivated) {
    // Templates
    using namespace blocksci;
    
    static std::vector<std::pair<AddressType::Enum, CScript>> mTemplates;
    if (mTemplates.empty())
    {
        // Standard tx, sender provides pubkey, receiver adds signature
        auto pubkey = std::make_pair(AddressType::Enum::PUBKEY, CScript() << OP_PUBKEY << OP_CHECKSIG);
        mTemplates.push_back(pubkey);
        
        // Bitcoin address tx, sender provides hash of pubkey, receiver provides signature and pubkey
        auto pubkeyHash = std::make_pair(AddressType::Enum::PUBKEYHASH, CScript() << OP_DUP << OP_HASH160 << OP_PUBKEYHASH << OP_EQUALVERIFY << OP_CHECKSIG);
        mTemplates.push_back(pubkeyHash);
        
        // Sender provides N pubkeys, receivers provides M signatures
        auto multisig = std::make_pair(AddressType::Enum::MULTISIG, CScript() << OP_SMALLINTEGER << OP_PUBKEYS << OP_SMALLINTEGER << OP_CHECKMULTISIG);
        mTemplates.push_back(multisig);
    }
    
    // Shortcut for pay-to-script-hash, which are more constrained than the other types:
    // it is always OP_HASH160 20 [20 byte hash] OP_EQUAL
    if (scriptPubKey.IsPayToScriptHash()) {
        blocksci::uint160 hash;
        memcpy(&hash, &(*(scriptPubKey.begin()+2)), 20);
        return ScriptData<AddressType::Enum::SCRIPTHASH>{{hash}};
    }
    
    if (witnessActivated && scriptPubKey.IsWitnessProgram()) {
        auto pc = scriptPubKey.begin();
        opcodetype opcode;
        boost::iterator_range<const unsigned char *> vchSig;
        scriptPubKey.GetOp(pc, opcode, vchSig);
        uint8_t version = static_cast<uint8_t>(CScript::DecodeOP_N(opcode));
        scriptPubKey.GetOp(pc, opcode, vchSig);
        if (version == 0 && vchSig.size() == 20) {
            return ScriptData<AddressType::Enum::WITNESS_PUBKEYHASH>(uint160{vchSig});
        } else if (version == 0 && vchSig.size() == 32) {
            return ScriptData<AddressType::Enum::WITNESS_SCRIPTHASH>(uint256{vchSig});
        }
    }
    
    // Provably prunable, data-carrying output
    //
    // So long as script passes the IsUnspendable() test and all but the first
    // byte passes the IsPushOnly() test we don't care what exactly is in the
    // script.
    
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
        return ScriptData<AddressType::Enum::NULL_DATA>{scriptPubKey};
    }
    
    // Scan templates
    const CScriptView& script1 = scriptPubKey;
    
    
    boost::optional<ScriptDataType> type;
    for (auto &tplate : mTemplates) {
        uint8_t numRequired = 0;
        bool isFirstSmallInt = true;
        
        const CScript& script2 = tplate.second;
        
        opcodetype opcode1, opcode2;
        boost::iterator_range<const unsigned char *> vch1;
        std::vector<unsigned char> vch2;
        
        // Compare
        CScriptView::const_iterator pc1 = script1.begin();
        CScript::const_iterator pc2 = script2.begin();
        while (true)
        {
            if (pc1 == script1.end() && pc2 == script2.end()) {
                
                if (!boost::apply_visitor([&](auto &data) { return data.isValid(); }, *type)) {
                    break;
                }
                return *type;
            }
            if (!script1.GetOp(pc1, opcode1, vch1))
                break;
            if (!script2.GetOp(pc2, opcode2, vch2))
                break;
            
            // Template matching opcodes:
            if (opcode2 == OP_PUBKEYS)
            {
                ScriptData<AddressType::Enum::MULTISIG> output;
                output.numRequired = numRequired;
                
                while (vch1.size() >= 33 && vch1.size() <= 65)
                {
                    output.addAddress(vch1);
                    
                    if (!script1.GetOp(pc1, opcode1, vch1))
                        break;
                }
                if (!script2.GetOp(pc2, opcode2, vch2))
                    break;
                
                type = output;
            }
            
            if (opcode2 == OP_PUBKEY)
            {
                if (!isValidPubkey(vch1)) {
                    break;
                }
                type = ScriptData<AddressType::Enum::PUBKEY>{vch1};
            }
            else if (opcode2 == OP_PUBKEYHASH)
            {
                if (vch1.size() != sizeof(uint160))
                    break;
                auto address = uint160{vch1};
                type = ScriptData<AddressType::Enum::PUBKEYHASH>{address};
            }
            else if (opcode2 == OP_SMALLINTEGER)
            {   // Single-byte small integer pushed onto vSolutions
                if (opcode1 == OP_0 || (opcode1 >= OP_1 && opcode1 <= OP_16)) {
                    if (isFirstSmallInt) {
                        numRequired = static_cast<uint8_t>(CScript::DecodeOP_N(opcode1));
                        isFirstSmallInt = false;
                    } else {
                        auto &out = boost::get<ScriptData<AddressType::Enum::MULTISIG>>(*type);
                        out.numTotal = static_cast<uint8_t>(CScript::DecodeOP_N(opcode1));
                    }
                }
                else
                    break;
            }
            else if (opcode1 != opcode2 || vch1 != vch2)
            {
                // Others must match exactly
                break;
            }
        }
    }
    return ScriptData<AddressType::Enum::NONSTANDARD>{scriptPubKey};
}

AnyScriptOutput::AnyScriptOutput(const blocksci::CScriptView &scriptPubKey, bool witnessActivated) : wrapped(extractScriptData(scriptPubKey, witnessActivated)) {}

blocksci::Address AnyScriptOutput::address() const {
    return boost::apply_visitor([&](auto &output) { return blocksci::Address{output.scriptNum, output.address_v}; }, wrapped);
}

bool AnyScriptOutput::isNew() const {
    return boost::apply_visitor([&](auto &output) { return output.isNew; }, wrapped);
}

bool AnyScriptOutput::isValid() const {
    return boost::apply_visitor([&](auto &output) { return output.data.isValid(); }, wrapped);
}

blocksci::AddressType::Enum AnyScriptOutput::type() const {
    return boost::apply_visitor([&](auto &output) { return output.address_v; }, wrapped);
}

void AnyScriptOutput::check(const AddressState &state) {
    boost::apply_visitor([&](auto &output) { return output.check(state); }, wrapped);
}

uint32_t AnyScriptOutput::resolve(AddressState &state) {
    return boost::apply_visitor([&](auto &output) { return output.resolve(state); }, wrapped);
}

// MARK: TX_PUBKEY

ScriptData<blocksci::AddressType::Enum::PUBKEY>::ScriptData(const boost::iterator_range<const unsigned char *> &vch1) : pubkey(vch1.begin(), vch1.end()) {}

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::PUBKEY>::getHash() const {
    return pubkey.GetID();
}

blocksci::PubkeyData ScriptData<blocksci::AddressType::Enum::PUBKEY>::getData() const {
    return {pubkey, pubkey.GetID()};
}

// MARK: TX_PUBKEYHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::PUBKEYHASH>::getHash() const {
    return hash;
}

blocksci::PubkeyData ScriptData<blocksci::AddressType::Enum::PUBKEYHASH>::getData() const {
    constexpr static CPubKey nullPubkey{};
    return {nullPubkey, hash};
}

// MARK: WITNESS_PUBKEYHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getHash() const {
    return hash;
}

blocksci::PubkeyData ScriptData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getData() const {
    constexpr static CPubKey nullPubkey{};
    return {nullPubkey, hash};
}

// MARK: TX_SCRIPTHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::SCRIPTHASH>::getHash() const {
    return hash;
}

blocksci::ScriptHashData ScriptData<blocksci::AddressType::Enum::SCRIPTHASH>::getData() const {
    blocksci::Address wrappedAddress;
    return {hash, wrappedAddress, 0};
}

// MARK: WITNESS_SCRIPTHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getHash() const {
    return ripemd160(reinterpret_cast<const char *>(&hash), sizeof(hash));
}

blocksci::ScriptHashData ScriptData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getData() const {
    blocksci::Address wrappedAddress;
    return {getHash(), wrappedAddress, 0};
}

// MARK: TX_MULTISIG

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::MULTISIG>::getHash() const {
    std::vector<char> sigData;
    sigData.resize(sizeof(numRequired) + sizeof(CKeyID) * addressCount);
    size_t sigDataPos = 0;
    
    memcpy(&sigData[sigDataPos], reinterpret_cast<const char *>(&numRequired), sizeof(numRequired));
    sigDataPos += sizeof(numRequired);
    
    std::vector<CPubKey> pubkeys;
    pubkeys.reserve(addresses.size());
    for (auto &output : addresses) {
        pubkeys.push_back(output.data.pubkey);
    }
    
    std::sort(pubkeys.begin(), pubkeys.end());
    
    for (auto &pubkey : pubkeys) {
        auto addressHash = pubkey.GetID();
        memcpy(&sigData[sigDataPos], reinterpret_cast<const char *>(&addressHash), sizeof(addressHash));
        sigDataPos += sizeof(addressHash);
    }
    
    return ripemd160(sigData.data(), sigData.size());
}

void ScriptData<blocksci::AddressType::Enum::MULTISIG>::addAddress(const boost::iterator_range<const unsigned char *> &vch1) {
    addresses.push_back(ScriptData<blocksci::AddressType::Enum::PUBKEY>(CPubKey(vch1.begin(), vch1.end())));
    addressCount++;
}

blocksci::ArbitraryLengthData<blocksci::MultisigData> ScriptData<blocksci::AddressType::Enum::MULTISIG>::getData() const {
    blocksci::MultisigData multisigData{numRequired, numTotal, addressCount};
    blocksci::ArbitraryLengthData<blocksci::MultisigData> data(multisigData);
    for (auto &pubkeyScript : addresses) {
        data.add(pubkeyScript.scriptNum);
    }
    return data;
}

// MARK: TX_NONSTANDARD

ScriptData<blocksci::AddressType::Enum::NONSTANDARD>::ScriptData(const blocksci::CScriptView &script_) : script(script_) {}

blocksci::ArbitraryLengthData<blocksci::NonstandardScriptData> ScriptData<blocksci::AddressType::Enum::NONSTANDARD>::getData() const {
    blocksci::NonstandardScriptData scriptData(script);
    blocksci::ArbitraryLengthData<blocksci::NonstandardScriptData> data(scriptData);
    data.add(script.begin(), script.end());
    return data;
}

// MARK: TX_NULL_DATA

ScriptData<blocksci::AddressType::Enum::NULL_DATA>::ScriptData(const blocksci::CScriptView &script){
    blocksci::CScriptView::const_iterator pc1 = script.begin();
    opcodetype opcode1;
    boost::iterator_range<const unsigned char *> vch1;
    while(true) {
        if(!script.GetOp(pc1, opcode1, vch1)) {
            break;
        }
        fullData.insert(fullData.end(), vch1.begin(), vch1.end());
    }
}

blocksci::ArbitraryLengthData<blocksci::RawData> ScriptData<blocksci::AddressType::Enum::NULL_DATA>::getData() const {
    blocksci::RawData scriptData(fullData);
    blocksci::ArbitraryLengthData<blocksci::RawData> data(scriptData);
    data.add(fullData.begin(), fullData.end());
    return data;
}
