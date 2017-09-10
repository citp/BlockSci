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

#include "blockchain_state.hpp"

#include <blocksci/hash.hpp>

#include <boost/variant/get.hpp>

template <bool deduped>
struct AddressNumImp {
    template <blocksci::AddressType::Enum type>
    static std::pair<blocksci::Address, bool> get(ScriptOutput<type> &data, BlockchainState &state);
    
    template <blocksci::AddressType::Enum type>
    static std::pair<blocksci::Address, bool> check(ScriptOutput<type> &data, const BlockchainState &state);
};

template <>
struct AddressNumImp<true> {
    template <blocksci::AddressType::Enum type>
    static std::pair<blocksci::Address, bool> get(ScriptOutput<type> &data, BlockchainState &state) {
        blocksci::RawAddress rawAddress{data.getHash(), blocksci::scriptType(type)};
        auto addressInfo = state.findAddress(rawAddress);
        auto processed = state.resolveAddress(addressInfo);
        return std::make_pair(blocksci::Address(processed.first, type), processed.second);
    }
    
    template <blocksci::AddressType::Enum type>
    static std::pair<blocksci::Address, bool> check(ScriptOutput<type> &data, const BlockchainState &state) {
        blocksci::RawAddress rawAddress{data.getHash(), blocksci::scriptType(type)};
        auto addressInfo = state.findAddress(rawAddress);
        return std::make_pair(blocksci::Address(addressInfo.addressNum, type), addressInfo.addressNum == 0);
    }
};

template <>
struct AddressNumImp<false> {
    template <blocksci::AddressType::Enum type>
    static std::pair<blocksci::Address, bool> get(ScriptOutput<type> &, BlockchainState &state) {
        auto index = state.getNewAddressIndex(scriptType(type));
        return std::make_pair(blocksci::Address(index, type), true);
    }
    
    template <blocksci::AddressType::Enum type>
    static std::pair<blocksci::Address, bool> check(ScriptOutput<type> &, const BlockchainState &) {
        return std::make_pair(blocksci::Address(0, type), true);
    }
};


template <blocksci::AddressType::Enum type>
std::pair<blocksci::Address, bool> getAddressNum(ScriptOutput<type> &data, BlockchainState &state) {
    return AddressNumImp<blocksci::AddressInfo<type>::deduped>::get(data, state);
}

template <blocksci::AddressType::Enum type>
std::pair<blocksci::Address, bool> checkAddressNum(ScriptOutput<type> &data, const BlockchainState &state) {
    return AddressNumImp<blocksci::AddressInfo<type>::deduped>::check(data, state);
}

#define VAL(x) template std::pair<blocksci::Address, bool> getAddressNum<blocksci::AddressType::Enum::x>(ScriptOutput<blocksci::AddressType::Enum::x> &data, BlockchainState &state);
ADDRESS_TYPE_SET
#undef VAL

#define VAL(x) template std::pair<blocksci::Address, bool> checkAddressNum<blocksci::AddressType::Enum::x>(ScriptOutput<blocksci::AddressType::Enum::x> &data, const BlockchainState &state);
ADDRESS_TYPE_SET
#undef VAL

struct ScriptOutputIsValid : public boost::static_visitor<bool> {
    template <blocksci::AddressType::Enum type>
    bool operator()(const ScriptOutput<type> &data) const {
        return data.isValid();
    }
};

bool isValid(const ScriptOutputType &type) {
    static auto visitor = ScriptOutputIsValid();
    return boost::apply_visitor(visitor, type);
}

// MARK: TX_PUBKEY

ScriptOutput<blocksci::AddressType::Enum::PUBKEY>::ScriptOutput(const std::vector<unsigned char> &vch1) : pubkey(vch1) {}

blocksci::uint160 ScriptOutput<blocksci::AddressType::Enum::PUBKEY>::getHash() {
    return pubkey.GetID();
}

// MARK: TX_PUBKEYHASH

blocksci::uint160 ScriptOutput<blocksci::AddressType::Enum::PUBKEYHASH>::getHash() {
    return hash;
}

// MARK: WITNESS_PUBKEYHASH

blocksci::uint160 ScriptOutput<blocksci::AddressType::Enum::WITNESS_PUBKEYHASH>::getHash() {
    return hash;
}

// MARK: TX_SCRIPTHASH

blocksci::uint160 ScriptOutput<blocksci::AddressType::Enum::SCRIPTHASH>::getHash() {
    return hash;
}

// MARK: WITNESS_SCRIPTHASH

blocksci::uint160 ScriptOutput<blocksci::AddressType::Enum::WITNESS_SCRIPTHASH>::getHash() {
    return ripemd160(reinterpret_cast<const char *>(&hash), sizeof(hash));
}

// MARK: TX_MULTISIG

blocksci::uint160 ScriptOutput<blocksci::AddressType::Enum::MULTISIG>::getHash() {
    std::vector<char> sigData;
    sigData.resize(sizeof(numRequired) + sizeof(CKeyID) * addressCount);
    size_t sigDataPos = 0;
    
    memcpy(&sigData[sigDataPos], reinterpret_cast<const char *>(&numRequired), sizeof(numRequired));
    sigDataPos += sizeof(numRequired);
    
    auto sortedAddresses = addresses;
    std::sort(sortedAddresses.begin(), sortedAddresses.begin() + addressCount);
    
    for (int i = 0; i < addressCount; i++) {
        auto addressHash = sortedAddresses[i].GetID();
        memcpy(&sigData[sigDataPos], reinterpret_cast<const char *>(&addressHash), sizeof(addressHash));
        sigDataPos += sizeof(addressHash);
    }
    
    return ripemd160(sigData.data(), sigData.size());
}

void ScriptOutput<blocksci::AddressType::Enum::MULTISIG>::processOutput(BlockchainState &state) {
    for (int i = 0; i < addressCount; i++) {
        blocksci::RawAddress rawAddress{addresses[i].GetID(), blocksci::ScriptType::Enum::PUBKEY};
        auto addressInfo = state.findAddress(rawAddress);
        auto addrGetRes = state.resolveAddress(addressInfo);
        processedAddresses[i] = addrGetRes.first;
        firstSeen[i] = addrGetRes.second;
    }
}

void ScriptOutput<blocksci::AddressType::Enum::MULTISIG>::checkOutput(const BlockchainState &state) {
    for (int i = 0; i < addressCount; i++) {
        blocksci::RawAddress rawAddress{addresses[i].GetID(), blocksci::ScriptType::Enum::PUBKEY};
        auto addressInfo = state.findAddress(rawAddress);
        processedAddresses[i] = addressInfo.addressNum;
        firstSeen[i] = addressInfo.addressNum == 0;
    }
}

void ScriptOutput<blocksci::AddressType::Enum::MULTISIG>::addAddress(const std::vector<unsigned char> &vch1) {
    addresses[addressCount] = CPubKey(vch1);
    addressCount++;
}

// MARK: TX_NONSTANDARD

ScriptOutput<blocksci::AddressType::Enum::NONSTANDARD>::ScriptOutput(const CScript &script_) : script(script_) {}

// MARK: TX_NULL_DATA

ScriptOutput<blocksci::AddressType::Enum::NULL_DATA>::ScriptOutput(const CScript &script){
    CScript::const_iterator pc1 = script.begin();
    opcodetype opcode1;
    std::vector<unsigned char> vch1;
    while(true) {
        if(!script.GetOp(pc1, opcode1, vch1)) {
            break;
        }
        fullData.insert(fullData.end(), vch1.begin(), vch1.end());
    }
}

// MARK: Script Processing

bool isValidPubkey(std::vector<unsigned char> &vch1) {
    if (vch1.size() < 33 || vch1.size() > 65)
        return false;
    
    char chHeader = vch1[0];
    if ((chHeader == 2 || chHeader == 3) && vch1.size() == 33) {
        return true;
    } else if ((chHeader == 4 || chHeader == 6 || chHeader == 7) && vch1.size() == 65) {
        return true;
    }
    
    return false;
}

ScriptOutputType extractScriptData(const unsigned char *scriptBegin, const unsigned char *scriptEnd) {
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
    CScript scriptPubKey(scriptBegin, scriptEnd);
    
    // Shortcut for pay-to-script-hash, which are more constrained than the other types:
    // it is always OP_HASH160 20 [20 byte hash] OP_EQUAL
    if (scriptPubKey.IsPayToScriptHash())
    {
        blocksci::uint160 hash;
        memcpy(&hash, &(*(scriptPubKey.begin()+2)), 20);
        return ScriptOutput<AddressType::Enum::SCRIPTHASH>{{hash}};
    }
    
    if (scriptPubKey.IsWitnessProgram()) {
        auto pc = scriptPubKey.begin();
        opcodetype opcode;
        std::vector<unsigned char> vchSig;
        scriptPubKey.GetOp(pc, opcode, vchSig);
        uint8_t version = CScript::DecodeOP_N(opcode);
        scriptPubKey.GetOp(pc, opcode, vchSig);
        if (version == 0 && vchSig.size() == 20) {
            return ScriptOutput<AddressType::Enum::WITNESS_PUBKEYHASH>(uint160{vchSig});
        } else if (version == 0 && vchSig.size() == 32) {
            return ScriptOutput<AddressType::Enum::WITNESS_SCRIPTHASH>(uint256{vchSig});
        }
        
//
//        
//        if (version == 0 && scriptPubKey.size() == 22) {
//
//            typeRet = TX_WITNESS_V0_KEYHASH;
//            vSolutionsRet.push_back(witnessprogram);
//            return true;
//        }
//        if (witnessversion == 0 && witnessprogram.size() == 34) {
//            typeRet = TX_WITNESS_V0_SCRIPTHASH;
//            vSolutionsRet.push_back(witnessprogram);
//            return true;
//        }
//        return false;
    }
    
    // Provably prunable, data-carrying output
    //
    // So long as script passes the IsUnspendable() test and all but the first
    // byte passes the IsPushOnly() test we don't care what exactly is in the
    // script.
    
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
        return ScriptOutput<AddressType::Enum::NULL_DATA>{scriptPubKey};
    }
    
    // Scan templates
    const CScript& script1 = scriptPubKey;
    
    
    boost::optional<ScriptOutputType> type;
    for (auto &tplate : mTemplates) {
        uint8_t numRequired = 0;
        bool isFirstSmallInt = true;
        
        const CScript& script2 = tplate.second;
        
        opcodetype opcode1, opcode2;
        std::vector<unsigned char> vch1, vch2;
        
        // Compare
        CScript::const_iterator pc1 = script1.begin();
        CScript::const_iterator pc2 = script2.begin();
        while (true)
        {
            if (pc1 == script1.end() && pc2 == script2.end())
            {
                if (!isValid(*type)) {
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
                ScriptOutput<AddressType::Enum::MULTISIG> output;
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
                type = ScriptOutput<AddressType::Enum::PUBKEY>{vch1};
            }
            else if (opcode2 == OP_PUBKEYHASH)
            {
                if (vch1.size() != sizeof(uint160))
                    break;
                auto address = uint160{vch1};
                type = ScriptOutput<AddressType::Enum::PUBKEYHASH>{address};
            }
            else if (opcode2 == OP_SMALLINTEGER)
            {   // Single-byte small integer pushed onto vSolutions
                if (opcode1 == OP_0 || (opcode1 >= OP_1 && opcode1 <= OP_16)) {
                    if (isFirstSmallInt) {
                        numRequired = (char)CScript::DecodeOP_N(opcode1);
                        isFirstSmallInt = false;
                    } else {
                        auto &out = boost::get<ScriptOutput<AddressType::Enum::MULTISIG>>(*type);
                        out.numTotal = (char)CScript::DecodeOP_N(opcode1);
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
    return ScriptOutput<AddressType::Enum::NONSTANDARD>{scriptPubKey};
}
