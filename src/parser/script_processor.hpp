//
//  script_processor.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 10/7/17.
//

#ifndef script_processor_hpp
#define script_processor_hpp

#include <blocksci/scripts/bitcoin_script.hpp>

#include <boost/range/iterator_range.hpp>

#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

/** Serialized script, used inside transaction inputs and outputs */
class CScriptView : public boost::iterator_range<const unsigned char *>  {
public:
    CScriptView() : boost::iterator_range<const unsigned char *>() {}
    CScriptView(const unsigned char* pbegin, const unsigned char* pend) : boost::iterator_range<const unsigned char *>(pbegin, pend) {}
    
    bool GetOp(const_iterator& pc, opcodetype& opcodeRet, boost::iterator_range<const unsigned char *>& vchRet) const
    {
        return GetOp2(pc, opcodeRet, &vchRet);
    }
    
    bool GetOp(const_iterator& pc, opcodetype& opcodeRet) const
    {
        return GetOp2(pc, opcodeRet, nullptr);
    }
    
    bool GetOp2(const_iterator& pc, opcodetype& opcodeRet, boost::iterator_range<const unsigned char *>* pvchRet) const
    {
        opcodeRet = OP_INVALIDOPCODE;
        if (pvchRet)
            *pvchRet = boost::iterator_range<const unsigned char *>();
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
                nSize = *reinterpret_cast<const uint16_t *>(&pc[0]);
                pc += 2;
            }
            else if (opcode == OP_PUSHDATA4)
            {
                if (end() - pc < 4)
                    return false;
                nSize = *reinterpret_cast<const uint32_t *>(&pc[0]);
                pc += 4;
            }
            if (end() - pc < 0 || static_cast<unsigned int>(end() - pc) < nSize)
                return false;
            if (pvchRet)
                *pvchRet = boost::iterator_range<const unsigned char *>(pc, pc + nSize);
            pc += nSize;
        }
        
        opcodeRet = static_cast<opcodetype>(opcode);
        return true;
    }
    
    int Find(opcodetype op) const
    {
        int nFound = 0;
        opcodetype opcode = OP_0;
        for (const_iterator pc = begin(); pc != end() && GetOp(pc, opcode);)
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
    bool IsWitnessProgram(int& version, std::vector<unsigned char>& program) const;
    bool IsWitnessProgram() const;
    
    /** Called by IsStandardTx and P2SH/BIP62 VerifyScript (which makes it consensus-critical). */
    bool IsPushOnly(const_iterator pc) const;
    bool IsPushOnly() const;
    
    /** Check if the script contains valid OP_CODES */
    bool HasValidOps() const;
    
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

#endif /* script_processor_hpp */
