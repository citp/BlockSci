//
//  script_view.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/7/17.
//

#include "script_view.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>

#include <sstream>

namespace blocksci {
    unsigned int CScriptView::GetSigOpCount(bool fAccurate) const
    {
        unsigned int n = 0;
        iterator pc = begin();
        opcodetype lastOpcode = OP_INVALIDOPCODE;
        while (pc < end())
        {
            opcodetype opcode;
            if (!GetOp(pc, opcode)) {
                break;
            }
            if (opcode == OP_CHECKSIG || opcode == OP_CHECKSIGVERIFY) {
                n++;
            } else if (opcode == OP_CHECKMULTISIG || opcode == OP_CHECKMULTISIGVERIFY) {
                if (fAccurate && lastOpcode >= OP_1 && lastOpcode <= OP_16) {
                    n += static_cast<unsigned int>(CScript::DecodeOP_N(lastOpcode));
                } else {
                    n += MAX_PUBKEYS_PER_MULTISIG;
                }
            }
            lastOpcode = opcode;
        }
        return n;
    }
    
    unsigned int CScriptView::GetSigOpCount(const CScriptView& scriptSig) const
    {
        if (!IsPayToScriptHash()) {
            return GetSigOpCount(true);
        }
        
        // This is a pay-to-script-hash scriptPubKey;
        // get the last item that the scriptSig
        // pushes onto the stack:
        iterator pc = scriptSig.begin();
        ranges::subrange<const unsigned char *> vData;
        while (pc < scriptSig.end()) {
            opcodetype opcode;
            if (!scriptSig.GetOp(pc, opcode, vData)) {
                return 0;
            }
            if (opcode > OP_16) {
                return 0;
            }
        }
        
        /// ... and return its opcount:
        CScriptView subscript(vData.begin(), vData.end());
        return subscript.GetSigOpCount(true);
    }
    
    bool CScriptView::IsPayToScriptHash() const {
        // Extra-fast test for pay-to-script-hash CScripts:
        return (this->size() == 23 &&
                (*this)[0] == OP_HASH160 &&
                (*this)[1] == 0x14 &&
                (*this)[22] == OP_EQUAL);
    }
    
    bool CScriptView::IsPayToWitnessScriptHash() const {
        // Extra-fast test for pay-to-witness-script-hash CScripts:
        return (this->size() == 34 &&
                (*this)[0] == OP_0 &&
                (*this)[1] == 0x20);
    }
    
    // A witness program is any valid CScript that consists of a 1-byte push opcode
    // followed by a data push between 2 and 40 bytes.
    bool CScriptView::IsWitnessProgram(uint8_t& version, ranges::subrange<const unsigned char *>& program) const {
        if (this->size() < 4 || this->size() > 42) {
            return false;
        }
        if ((*this)[0] != OP_0 && ((*this)[0] < OP_1 || (*this)[0] > OP_16)) {
            return false;
        }
        if (static_cast<size_t>((*this)[1] + 2) == this->size()) {
            version = CScript::DecodeOP_N(static_cast<opcodetype>((*this)[0]));
            program = {this->begin() + 2, this->end()};
            return true;
        }
        return false;
    }
    
    bool CScriptView::IsWitnessProgram() const {
        if (this->size() < 4 || this->size() > 42) {
            return false;
        }
        if ((*this)[0] != OP_0 && ((*this)[0] < OP_1 || (*this)[0] > OP_16)) {
            return false;
        }
        return (static_cast<size_t>((*this)[1] + 2) == this->size());
    }
    
    bool CScriptView::IsPushOnly(iterator pc) const {
        while (pc < end()) {
            opcodetype opcode;
            if (!GetOp(pc, opcode)) {
                return false;
            }
            // Note that IsPushOnly() *does* consider OP_RESERVED to be a
            // push-type opcode, however execution of OP_RESERVED fails, so
            // it's not relevant to P2SH/BIP62 as the scriptSig would fail prior to
            // the P2SH special validation code being executed.
            if (opcode > OP_16) {
                return false;
            }
        }
        return true;
    }
    
    bool CScriptView::IsPushOnly() const {
        return this->IsPushOnly(begin());
    }
    
    std::string ScriptToAsmStr(const CScriptView& script, const bool fAttemptSighashDecode) {
        std::stringstream ss;
        opcodetype opcode;
        ranges::subrange<const unsigned char *> vch;
        CScriptView::iterator pc = script.begin();
        while (pc < script.end()) {
            if (!ss.str().empty()) {
                ss << " ";
            }
            if (!script.GetOp(pc, opcode, vch)) {
                ss << "[error]";
                return ss.str();
            }
            if (0 <= opcode && opcode <= OP_PUSHDATA4) {
                if (vch.size() <= static_cast<std::vector<unsigned char>::size_type>(4)) {
                    ss << CScriptNum(vch | ranges::to_vector, false).getint();
                } else {
                    // the IsUnspendable check makes sure not to try to decode OP_RETURN data that may match the format of a signature
                    if (fAttemptSighashDecode && !script.IsUnspendable()) {
                        std::string strSigHashDecode;
                        ss << HexStr(vch) << strSigHashDecode;
                    } else {
                        ss << HexStr(vch);
                    }
                }
            } else {
                ss << GetOpName(opcode);
            }
        }
        return ss.str();
    }
} // namespace blocksci

