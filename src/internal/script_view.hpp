//
//  script_view.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/7/17.
//

#ifndef script_view_hpp
#define script_view_hpp

#include "bitcoin_script.hpp"

#include <range/v3/view/subrange.hpp>

#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace blocksci {
    
    /** Serialized script, used inside transaction inputs and outputs */
    class CScriptView : public ranges::subrange<const unsigned char *>  {
    public:
        CScriptView() : ranges::subrange<const unsigned char *>() {}
        CScriptView(const unsigned char* pbegin, const unsigned char* pend) : ranges::subrange<const unsigned char *>(pbegin, pend) {}
        
        bool GetOp(iterator& pc, opcodetype& opcodeRet, ranges::subrange<const unsigned char *>& vchRet) const
        {
            return GetOp2(pc, opcodeRet, &vchRet);
        }
        
        bool GetOp(iterator& pc, opcodetype& opcodeRet) const
        {
            return GetOp2(pc, opcodeRet, nullptr);
        }
        
        bool GetOp2(iterator& pc, opcodetype& opcodeRet, ranges::subrange<const unsigned char *>* pvchRet) const
        {
            opcodeRet = OP_INVALIDOPCODE;
            if (pvchRet)
                *pvchRet = ranges::subrange<const unsigned char *>();
            if (pc >= end())
                return false;
            
            // Read instruction
            if (end() - pc < 1)
                return false;
            unsigned int opcode = *pc++;
            
            // Immediate operand
            if (opcode <= OP_PUSHDATA4)
            {
                unsigned int nSize = 0;
                if (opcode < OP_PUSHDATA1)
                {
                    nSize = opcode;
                }
                else if (opcode == OP_PUSHDATA1)
                {
                    if (end() - pc < 1)
                        return false;
                    nSize = *pc++;
                }
                else if (opcode == OP_PUSHDATA2)
                {
                    if (end() - pc < 2)
                        return false;
                    uint16_t sizeVal;
                    std::memcpy(&sizeVal, &pc[0], sizeof(sizeVal));
                    nSize = sizeVal;
                    pc += 2;
                }
                else if (opcode == OP_PUSHDATA4)
                {
                    if (end() - pc < 4)
                        return false;
                    uint32_t sizeVal;
                    std::memcpy(&sizeVal, &pc[0], sizeof(sizeVal));
                    nSize = sizeVal;
                    pc += 4;
                }
                if (end() - pc < 0 || static_cast<unsigned int>(end() - pc) < nSize)
                    return false;
                if (pvchRet)
                    *pvchRet = ranges::subrange<const unsigned char *>(pc, pc + nSize);
                pc += nSize;
            }
            
            opcodeRet = static_cast<opcodetype>(opcode);
            return true;
        }
        
        int Find(opcodetype op) const
        {
            int nFound = 0;
            opcodetype opcode = OP_0;
            for (iterator pc = begin(); pc != end() && GetOp(pc, opcode);)
                if (opcode == op)
                    ++nFound;
            return nFound;
        }
        
        /**
         * Pre-version-0.6, Bitcoin always counted CHECKMULTISIGs
         * as 20 sigops. With pay-to-script-hash, that changed:
         * CHECKMULTISIGs serialized in scriptSigs are
         * counted more accurately, assuming they are of the form
         *  ... OP_N CHECKMULTISIG ...
         */
        unsigned int GetSigOpCount(bool fAccurate) const;
        
        /**
         * Accurately count sigOps, including sigOps in
         * pay-to-script-hash transactions:
         */
        unsigned int GetSigOpCount(const CScriptView& scriptSig) const;
        
        bool IsPayToScriptHash() const;
        bool IsPayToWitnessScriptHash() const;
        bool IsWitnessProgram(uint8_t& version, ranges::subrange<const unsigned char *>& program) const;
        bool IsWitnessProgram() const;
        
        /** Called by IsStandardTx and P2SH/BIP62 VerifyScript (which makes it consensus-critical). */
        bool IsPushOnly(iterator pc) const;
        bool IsPushOnly() const;

        /**
         * Returns whether the script is guaranteed to fail at execution,
         * regardless of the initial stack. This allows outputs to be pruned
         * instantly when entering the UTXO set.
         */
        bool IsUnspendable() const
        {
            return (size() > 0 && *begin() == OP_RETURN) || (size() > MAX_SCRIPT_SIZE);
        }
    };
    
    std::string ScriptToAsmStr(const CScriptView& script, const bool fAttemptSighashDecode = false);
} // namespace blocksci

#endif /* script_view_hpp */
