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

struct IsValidVisitor : public boost::static_visitor<bool> {
    template <blocksci::AddressType::Enum type>
    bool operator()(const ScriptOutput<type> &scriptOutput) const {
        return scriptOutput.data.isValid();
    }
    
    template <blocksci::AddressType::Enum type>
    bool operator()(const ScriptData<type> &scriptOutput) const {
        return scriptOutput.isValid();
    }
};

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

ScriptDataType extractScriptData(const CScriptView &scriptPubKey, bool witnessActivated) {
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
                if (!boost::apply_visitor(IsValidVisitor(), *type)) {
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

struct AddressVisitor : public boost::static_visitor<blocksci::Address> {
    template <blocksci::AddressType::Enum type>
    blocksci::Address operator()(const ScriptOutput<type> &scriptOutput) {
        return {scriptOutput.scriptNum, type};
    }
};

struct IsNewVisitor : public boost::static_visitor<bool> {
    template <blocksci::AddressType::Enum type>
    bool operator()(const ScriptOutput<type> &scriptOutput) const {
        return scriptOutput.isNew;
    }
};

struct OutputAddressTypeVisitor : public boost::static_visitor<blocksci::AddressType::Enum> {
    template <blocksci::AddressType::Enum type>
    blocksci::AddressType::Enum operator()(const ScriptOutput<type> &) const {
        return type;
    }
};

struct CheckOutputVisitor : public boost::static_visitor<void> {
    const AddressState &state;
    CheckOutputVisitor(const AddressState &state_) : state(state_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(ScriptOutput<type> &scriptOutput) const {
        scriptOutput.check(state);
    }
};

struct ResolveOutputVisitor : public boost::static_visitor<void> {
    AddressState &state;
    ResolveOutputVisitor(AddressState &state_) : state(state_) {}
    template <blocksci::AddressType::Enum type>
    void operator()(ScriptOutput<type> &scriptOutput) const {
        scriptOutput.resolve(state);
    }
};

AnyScriptOutput::AnyScriptOutput(const CScriptView &scriptPubKey, bool witnessActivated) : wrapped(extractScriptData(scriptPubKey, witnessActivated)) {}

blocksci::Address AnyScriptOutput::address() const {
    AddressVisitor visitor;
    return boost::apply_visitor(visitor, wrapped);
}

bool AnyScriptOutput::isNew() const {
    IsNewVisitor visitor;
    return boost::apply_visitor(visitor, wrapped);
}

bool AnyScriptOutput::isValid() const {
    IsValidVisitor visitor;
    return boost::apply_visitor(visitor, wrapped);
}

blocksci::AddressType::Enum AnyScriptOutput::type() const {
    OutputAddressTypeVisitor visitor;
    return boost::apply_visitor(visitor, wrapped);
}

void AnyScriptOutput::check(const AddressState &state) {
    CheckOutputVisitor visitor{state};
    boost::apply_visitor(visitor, wrapped);
}

void AnyScriptOutput::resolve(AddressState &state) {
    ResolveOutputVisitor visitor{state};
    boost::apply_visitor(visitor, wrapped);
}


// MARK: TX_PUBKEY

ScriptData<blocksci::AddressType::Enum::PUBKEY>::ScriptData(const boost::iterator_range<const unsigned char *> &vch1) : pubkey(vch1.begin(), vch1.end()) {}

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::PUBKEY>::getHash() const {
    return pubkey.GetID();
}

// MARK: TX_PUBKEYHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::PUBKEYHASH>::getHash() const {
    return hash;
}

// MARK: WITNESS_PUBKEYHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getHash() const {
    return hash;
}

// MARK: TX_SCRIPTHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::SCRIPTHASH>::getHash() const {
    return hash;
}

// MARK: WITNESS_SCRIPTHASH

blocksci::uint160 ScriptData<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getHash() const {
    return ripemd160(reinterpret_cast<const char *>(&hash), sizeof(hash));
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
    
    std::sort(pubkeys.begin(), pubkeys.begin() + addressCount);
    
    for (auto &pubkey : pubkeys) {
        auto addressHash = pubkey.GetID();
        memcpy(&sigData[sigDataPos], reinterpret_cast<const char *>(&addressHash), sizeof(addressHash));
        sigDataPos += sizeof(addressHash);
    }
    
    return ripemd160(sigData.data(), sigData.size());
}

void ScriptData<blocksci::AddressType::Enum::MULTISIG>::resolve(AddressState &state) {
    for (auto &address : addresses) {
        address.resolve(state);
    }
}

void ScriptData<blocksci::AddressType::Enum::MULTISIG>::check(const AddressState &state) {
    for (auto &address : addresses) {
        address.check(state);
    }
}

void ScriptData<blocksci::AddressType::Enum::MULTISIG>::addAddress(const boost::iterator_range<const unsigned char *> &vch1) {
    addresses.push_back(ScriptData<blocksci::AddressType::Enum::PUBKEY>(CPubKey(vch1.begin(), vch1.end())));
    addressCount++;
}

// MARK: TX_NONSTANDARD

ScriptData<blocksci::AddressType::Enum::NONSTANDARD>::ScriptData(const CScriptView &script_) : script(script_) {}

// MARK: TX_NULL_DATA

ScriptData<blocksci::AddressType::Enum::NULL_DATA>::ScriptData(const CScriptView &script){
    CScriptView::const_iterator pc1 = script.begin();
    opcodetype opcode1;
    boost::iterator_range<const unsigned char *> vch1;
    while(true) {
        if(!script.GetOp(pc1, opcode1, vch1)) {
            break;
        }
        fullData.insert(fullData.end(), vch1.begin(), vch1.end());
    }
}
